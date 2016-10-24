/*
 * frame-buffer.cpp
 *
 *  Created on: Jun 15, 2016
 *      Author: xyh
 */

#include <iostream>

#include "utils.h"
#include "frame-buffer.h"
#include "namespacer.h"
#include "logger.hpp"

//*****************************************************
//  Segment
//*****************************************************
FrameBuffer::Slot::Segment::Segment()
{
    reset();
}

FrameBuffer::Slot::Segment::~Segment()
{}

void
FrameBuffer::Slot::Segment::interestIssued (const ndn::Interest& interest)
{
    uint32_t nonceValue = NdnRtcUtils::blobToNonce(interest.getNonce());

    assert(nonceValue != 0);

    state_ = StatePending;

    if (requestTimeUsec_ <= 0)
        requestTimeUsec_ = NdnRtcUtils::microsecondTimestamp();

    Name name = interest.getName();
    FrameNumber frameNo;
    SegmentNumber segNo;
    Namespacer::getSegmentationNumbers(name,frameNo,segNo);
    setSegmentNumber(segNo);

    interestNonce_ = nonceValue;
    requestCounter_++;
}

void
FrameBuffer::Slot::Segment::dataArrived (const SegmentData::SegmentMetaInfo& segmentMeta)
{
    state_ = StateFetched;
    dataNonce_ = segmentMeta.interestNonce_;
    arrivalTimeUsec_ = NdnRtcUtils::microsecondTimestamp();
    consumeTimeMs_ = segmentMeta.interestArrivalMs_;
    generationDelayMs_ = segmentMeta.generationDelayMs_;
}

void
FrameBuffer::Slot::Segment::markMissed()
{
    state_ = StateMissing;
}

void
FrameBuffer::Slot::Segment::discard()
{
    reset();
}


//protected functions
void
FrameBuffer::Slot::Segment::reset()
{
    state_ = StateNotUsed;
    segmentNumber_ = -1;
    dataNonce_ = 0;
    interestNonce_ = -1;

    requestTimeUsec_ = -1;
    arrivalTimeUsec_ = -1;
    consumeTimeMs_ = -1;
    generationDelayMs_ = -1;

    requestCounter_ = 0;
    dataPtr_ = NULL;
    payloadSize_ = -1;
    isParity_ = false;
}



//*****************************************************
//  Slot
//*****************************************************

FrameBuffer::Slot::Slot()
{
    reset();
}

FrameBuffer::Slot::~Slot()
{
    if (slotData_)
            free(slotData_);
}

bool
FrameBuffer::Slot::addInterest( const ndn::Interest &interest )
{
    Name name = interest.getName();

    SegmentNumber segNumber;
    FrameNumber frameNumber;

    Namespacer::getSegmentationNumbers(name,frameNumber,segNumber);

    ptr_lib::shared_ptr<Segment> segment;
    segment = prepareSegment( segNumber );    // get the segment by segment number

    if( segment->getState() == Segment::StateFetched )
        return false;
    segment->interestIssued( interest );

    // first time to express interest for this frame
    if ( getState() == StateFree )
    {
        state_ = StateNew;
        Namespacer::getFramePrefix(name,prefix_);
        frameNumber_ = frameNumber;
        requestTimeUsec_ = segment->getRequestTimeUsec();
    }

    nSegmentPending ++;

    return true;
}

void
FrameBuffer::Slot::appendData ( const ndn::Data &data )
{
    // extract prefix FrameNumber SegmentNumber prefixMetaInfo and SegmentData
    Name name = data.getName();
    int frameNo, segNo;
    Namespacer::getSegmentationNumbers(name,frameNo,segNo);

    PrefixMetaInfo prefixMetaInfo;
    Namespacer::getPrefixMetaInfo(name,prefixMetaInfo);
    SegmentData segmentData;
    SegmentData::segmentDataFromRaw(data.getContent().size(),
                                    data.getContent().buf(),
                                    segmentData);

    // get the Slot::Segment object by segNo
    ptr_lib::shared_ptr<Segment> segment;
    segment = getSegment( segNo );
    Segment::State segOldState = segment->getState();

    // update Slot
    nSegmentReady ++;
    segOldState == Segment::StateMissing ? nSegmentMissed -- : nSegmentPending --;
    if ( getState() == Slot::StateNew ) // first time to recieve segment for this frame
    {
        nSegmentTotal = prefixMetaInfo.totalSegmentNum_;
        firstSegmentTimeUsec_ = segment->getArrivalTimeUsec();
        updateSlot();   // update slot(segment number) according to prefixMetaInfo
        setState( StateAssembling ); // change to next state (StateAssembling)
    }

    // update Segment
    segment->setDataPtr(cacheData( segmentData, segNo ));//append data block to slotData_
    segment->setPayloadSize(data.getContent().size());
    segment->dataArrived( *(segmentData.getMetaData()) );
    segment->setState(Segment::StateFetched);

    if( nSegmentReady == nSegmentTotal )    //fetched all of segment for this frame
    {
        readyTimeUsec_ = segment->getArrivalTimeUsec();
        setState(Slot::StateReady);
    }

    cout  << "[FrameBuffer] Append Data " << name.toUri() << endl
          << " FrameNo:" << frameNo
          << " SegNo:" << segNo
          << " Total:" << nSegmentTotal
          << " Ready:" << nSegmentReady
          << " missing:" << nSegmentMissed
          << " pending:" << nSegmentPending
          << endl;
}

void
FrameBuffer::Slot::markMissed(const ndn::Interest &interest)
{
    Name name = interest.getName();
    FrameNumber frameNo;
    SegmentNumber segNo;
    Namespacer::getSegmentationNumbers(name, frameNo, segNo );
    ptr_lib::shared_ptr<Segment> segment;
    segment = getSegment(segNo);

    if( segment.get() )
    {
        if( segment->getState() == Segment::StateFetched )
            return ;
        segment->markMissed();
        if( segment->getState() == Segment::StatePending )
        {
            nSegmentMissed ++;
            nSegmentPending --;
        }
    }

    cout  << "[FrameBuffer] Missed " << name.toUri() << endl
          << " FrameNo:" << frameNo
          << " SegNo:" << segNo
          << " Total:" << nSegmentTotal
          << " Ready:" << nSegmentReady
          << " missing:" << nSegmentMissed
          << " pending:" << nSegmentPending
          << endl;
}

void
FrameBuffer::Slot::discard()
{
    reset();
}

// getter and setter
//**************************************************
void
FrameBuffer::Slot::getMissedSegments(std::vector<SegmentNumber>& missedSegments)
{
    std::map<SegmentNumber, ptr_lib::shared_ptr<Segment>>::iterator iter;
    ptr_lib::shared_ptr<Segment> segment;
    for ( iter = activeSegments_.begin(); iter != activeSegments_.end(); iter++ )
    {
        segment = iter->second;
        if( segment->getState() == Segment::StateMissing )
            missedSegments.push_back(segment->getSegmentNumber());
    }
}

//protected functions
//**************************************************
void
FrameBuffer::Slot::reset()
{
    state_ = StateFree;
    prefix_.clear();
    frameNumber_ = -1;
    payloadSize_ = -1;

    requestTimeUsec_ = -1;
    firstSegmentTimeUsec_ = -1;
    readyTimeUsec_ = -1;

    nSegmentMissed = 0;
    nSegmentTotal = 0;
    nSegmentPending = 0;
    nSegmentReady = 0;

    assembledSize_ = 0;
    payloadSize_ = 0;
    //memset( slotData_,0, allocatedSize_);

    // clear all active segments if any
    std::map<SegmentNumber, ptr_lib::shared_ptr<Segment> >::iterator iter;
    for( iter = activeSegments_.begin(); iter != activeSegments_.end(); iter++ )
    {
        iter->second->discard();
        freeSegments_.push_back(iter->second);
        activeSegments_.erase(iter);
    }
    activeSegments_.clear();
}

ptr_lib::shared_ptr<FrameBuffer::Slot::Segment>
FrameBuffer::Slot::pickFreeSegment()
{
    ptr_lib::shared_ptr<Segment> freeSegment;
    if( freeSegments_.size() )
    {
        freeSegment = freeSegments_.at(freeSegments_.size()-1);
        freeSegments_.pop_back();
    }
    else
    {
        freeSegment.reset(new Segment());
    }
    return freeSegment;
}

ptr_lib::shared_ptr<FrameBuffer::Slot::Segment>
FrameBuffer::Slot::prepareSegment(SegmentNumber segNo)
{
    ptr_lib::shared_ptr<Segment> freeSegment;
    std::map<SegmentNumber, ptr_lib::shared_ptr<Segment> >::iterator iter;

    iter = activeSegments_.find(segNo);
    // segment exist in activeSegments_
    if( iter != activeSegments_.end() )
    {
        freeSegment = iter->second;
    }
    //pick a free segment and set segment number with segNo
    else
    {
        freeSegment = pickFreeSegment();
        freeSegment->setSegmentNumber(segNo);
        activeSegments_[segNo] = freeSegment;
    }
    return freeSegment;
}

ptr_lib::shared_ptr<FrameBuffer::Slot::Segment>
FrameBuffer::Slot::getSegment(SegmentNumber segNo)
{
    ptr_lib::shared_ptr<Segment> segment;
    std::map<SegmentNumber, ptr_lib::shared_ptr<Segment> >::iterator iter;

    iter = activeSegments_.find(segNo);
    // segment exist in activeSegments_
    if( iter != activeSegments_.end() )
    {
        segment = iter->second;
    }
    return segment;
}

unsigned char*
FrameBuffer::Slot::cacheData( SegmentData segmentData, SegmentNumber segNo )
{
    // first time to allocate memory
    if( allocatedSize_ <= 0 )
    {
        slotData_ = (unsigned char*) malloc ( nSegmentTotal*segmentData.size() );
        allocatedSize_ = nSegmentTotal*segmentData.size();
    }
    // allocted memory is not enough
    if( allocatedSize_ - payloadSize_ < (unsigned int)segmentData.size() )
    {
        slotData_ = (unsigned char*) realloc ( slotData_, nSegmentTotal*segmentData.size() );
        allocatedSize_ = nSegmentTotal*segmentData.size();
    }

    // copy data
    int segIdx = segNo * segmentData.size();
    memcpy( slotData_+segIdx, segmentData.buf(), segmentData.size() );
    payloadSize_ += segmentData.size();

    cout << "Cache data" << endl
         << "segNo=" << segNo << endl
         << "alloc=" << allocatedSize_ << endl
         << "payload=" << payloadSize_ << endl
         << "start=" << segIdx << endl
         << "size=" << segmentData.size() << endl
         << endl;

    return slotData_+segIdx;
}

void
FrameBuffer::Slot::updateSlot()
{
    int requestSegments = nSegmentPending + nSegmentMissed;
    if( nSegmentTotal == requestSegments )
        return;
    // overestimate segment number, delete
    if( nSegmentTotal > requestSegments )
    {
        std::map<SegmentNumber, ptr_lib::shared_ptr<Segment>>::reverse_iterator rit;
        ptr_lib::shared_ptr<Segment> segment;

        rit = activeSegments_.rbegin();
        while( activeSegments_.size() > nSegmentTotal )
        //for ( it = activeSegments_.begin(); it != activeSegments_.end(); it++ )
        {
            segment = rit->second;
            if( segment->getSegmentNumber() >= nSegmentTotal )
            {
                if( segment->getState() == Segment::StateMissing )
                    nSegmentMissed --;
                if( segment->getState() == Segment::StatePending )
                    nSegmentPending --;
                freeActiveSegment(rit);
            }
            rit++;
        }
    }
    // underestimate segments number, mark Missing
    else
    {
        SegmentNumber segNo;
        for ( segNo = nSegmentPending; segNo < nSegmentTotal; segNo++ )
        {
            ptr_lib::shared_ptr<Segment> segment = pickFreeSegment();
            segment->setSegmentNumber(segNo);
            activeSegments_[segNo] = segment;
            segment->markMissed();
            segment->setState(Segment::StateMissing);
            nSegmentMissed++;
        }
    }
}

void
FrameBuffer::Slot::freeActiveSegment( SegmentNumber segNo )
{
    ptr_lib::shared_ptr<Segment> segment;
    std::map<SegmentNumber, ptr_lib::shared_ptr<Segment>>::iterator iter;
    iter = activeSegments_.find(segNo);
    if( iter!= activeSegments_.end() )
    {
        segment = iter->second;
        segment->discard();
        freeSegments_.push_back(segment);
        activeSegments_.erase(iter);
    }
}

void
FrameBuffer::Slot::freeActiveSegment
        (std::map<SegmentNumber, ptr_lib::shared_ptr<Segment>>::reverse_iterator iter )
{
    std::map<SegmentNumber, ptr_lib::shared_ptr<Segment>>::iterator it(iter.base());
    ptr_lib::shared_ptr<Segment> segment;
    segment = iter->second;
    segment->discard();
    freeSegments_.push_back(segment);
    activeSegments_.erase(it);
}



//*****************************************************
//  FrameBuffer
//*****************************************************

int
FrameBuffer::init( int slotNum )
{
    reset();
    initSlots(slotNum);
}

bool
FrameBuffer::interestIssued( const Interest& interest )
{
    lock_guard<recursive_mutex> scopedLock(syncMutex_);

    Name frameName;
    Namespacer::getFramePrefix(interest.getName(),frameName);

    //LOG(INFO) << "[FrameBuffer] Issued interest (" << frameName << ")";
    ptr_lib::shared_ptr<Slot> slot;
    slot = prepareSlot(frameName);
    if( slot.get() )
    {
        return slot->addInterest(interest);
    }

    return false;
}

void
FrameBuffer::recvData(const ndn::ptr_lib::shared_ptr<Data>& data)
{
    lock_guard<recursive_mutex> scopedLock(syncMutex_);

    Name name;
    Namespacer::getFramePrefix(data->getName(),name);
    FrameNumber frameNo;
    SegmentNumber segNo;
    Namespacer::getSegmentationNumbers(name,frameNo, segNo);

    ptr_lib::shared_ptr<Slot> slot = getSlot(name,false);
    slot->appendData(*(data.get()));

    checkRetransmissions();
}

void
FrameBuffer::interestTimeout(const ndn::Interest &interest)
{
    Name name = interest.getName();
    Name frameName;
    Namespacer::getFramePrefix(name,frameName);

    ptr_lib::shared_ptr<Slot> slot;
    slot = getSlot(frameName,false);
    slot->markMissed(interest);
    FrameNumber frameNo;
    SegmentNumber segNo;
    Namespacer::getSegmentationNumbers(interest.getName(), frameNo, segNo );
    if( slot->getState() != Slot::StateReady)
    //callback_->onSegmentNeeded(frameNo,segNo);
    callback_->onSegmentNeeded(name);
}

bool
FrameBuffer::acquireSlot ( MediaData &mediaData, PacketNumber packetNo )
{
    lock_guard<recursive_mutex> scopedLock(syncMutex_);

    ptr_lib::shared_ptr<Slot> slot;
    if( playbackQueue_.size() < 1 )
        return false;

    slot = playbackQueue_.top();
    if( slot.get() )
    {
        packetNo = slot->getFrameNumber();

        // request old frame
        if ( playbackNo_ >= 0
             && slot->getFrameNumber() >= 0
             && playbackNo_+1 != slot->getFrameNumber() )
        {
            //skip frame
            return false;
        }
        // not ready
        if ( slot->getState() != Slot::StateReady )
            return false;

        playbackSlot_ = slot;
        slot->lock();   // unlock in the FrameBuffer::releaseAcquiredSlot

        playbackNo_ = slot->getFrameNumber();
        mediaData.initFromRawData(slot->getPayloadSize(),slot->getDataPtr());

        return true;
    }
    return false;
}

void
FrameBuffer::releaseAcquiredSlot()
{
    playbackSlot_->unlock();    // lock in the FrameBuffer::acquireSlot
    playbackQueue_.pop();
    recycleOldSlot();
}

void
FrameBuffer::checkRetransmissions()
{
    std::vector<SegmentNumber> missedSegs;
    ptr_lib::shared_ptr<Slot> slot;
    std::map<Name, ptr_lib::shared_ptr<Slot>>::iterator iter;
    for ( iter = activeSlots_.begin(); iter!=activeSlots_.end(); iter++ )
    {
        slot = iter->second;
        if( slot->getState() != Slot::StateReady )
        {
            slot->getMissedSegments(missedSegs);
        }
    }
    if( !missedSegs.empty() )
    {
        vector<SegmentNumber>::iterator it;
        for( it = missedSegs.begin(); it != missedSegs.end(); it++)
            callback_->onSegmentNeeded(slot->getFrameNumber(),*it);
    }
}


//protected functions
//**************************************************
void
FrameBuffer::reset()
{
    lock_guard<recursive_mutex> scopedLock(syncMutex_);

    state_ = Invalid;
    playbackNo_ = -1;
    playbackSlot_.reset();
    callback_ = NULL;

    std::map<Name, ptr_lib::shared_ptr<Slot>>::iterator iter;
    ptr_lib::shared_ptr<Slot> slot;
    for ( iter = activeSlots_.begin(); iter != activeSlots_.end(); iter++ )
    {
        slot = iter->second;
        freeSlots_.push_back(slot);
    }
    activeSlots_.clear();
}

void
FrameBuffer::initSlots( int slotNum )
{
    while( freeSlots_.size() < slotNum )
    {
        ptr_lib::shared_ptr<Slot> slot(new Slot);
        freeSlots_.push_back(slot);
    }

    state_ = Valid;
}

ptr_lib::shared_ptr<FrameBuffer::Slot>
FrameBuffer::pickFreeSlot()
{
    ptr_lib::shared_ptr<Slot> freeSlot;
    freeSlot.reset();
    if( freeSlots_.size() )
    {
        freeSlot = freeSlots_.at(freeSlots_.size()-1);
        freeSlots_.pop_back();
    }
    return freeSlot;
}

ptr_lib::shared_ptr<FrameBuffer::Slot>
FrameBuffer::prepareSlot(const ndn::Name& prefix)
{
    ptr_lib::shared_ptr<Slot> slot;
    slot = getSlot(prefix);
    if ( !slot.get() )  // not in active slots
    {
        slot = pickFreeSlot();
        if( slot.get() )
        {
            slot->setPrefix(prefix);
            FrameNumber frameNo;
            Namespacer::getFrameNumber(prefix,frameNo);
            slot->setFrameNumber(frameNo);
            activeSlots_[prefix] = slot;
        }
    }
    return slot;
}

ptr_lib::shared_ptr<FrameBuffer::Slot>
FrameBuffer::getSlot(const ndn::Name& prefix, bool remove)
{
    ptr_lib::shared_ptr<Slot> slot;
    std::map<Name, ptr_lib::shared_ptr<Slot>>::iterator it;
    it = activeSlots_.find(prefix);
    if( it != activeSlots_.end() )
    {
        slot = it->second;
        if( remove )
        {
            freeSlots_.push_back(slot);
            activeSlots_.erase(it);
        }
    }
    return slot;
}

void
FrameBuffer::freeOldSlot()
{
    ptr_lib::shared_ptr<Slot> slot;
    slot = activeSlots_.begin()->second;
    if( slot.get() )
    {
        freeSlots_.push_back(slot);
        activeSlots_.erase(activeSlots_.begin());
    }
}

void
FrameBuffer::recycleOldSlot()
{
    ptr_lib::shared_ptr<Slot> slot = playbackQueue_.top();
    while( !playbackQueue_.empty() && slot->getFrameNumber() < playbackNo_ )
    {
        playbackQueue_.pop();
        freeOldSlot();
        slot = playbackQueue_.top();
    }
}
