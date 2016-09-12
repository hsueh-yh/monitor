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

//*****************************************************
//  Segment
//*****************************************************
FrameBuffer::Slot::Segment::Segment()
{
    reset();
}

FrameBuffer::Slot::Segment::~Segment()
{

}

void
FrameBuffer::Slot::Segment::interestIssued (const ndn::Interest& interest)
{
    uint32_t nonceValue = interest.getNonce();

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

void
FrameBuffer::Slot::addInterest( ndn::Interest &interest )
{
    Name name = interest.getName();
    SegmentNumber segNumber;
    FrameNumber frameNumber;

    Namespacer::getSegmentationNumbers(name,frameNumber,segNumber);

    boost::shared_ptr<Segment> segment;
    segment = prepareSegment( segNumber );    // get the segment by segment number

    segment->interestIssued( NdnRtcUtils::blobToNonce(interest.getNonce()) );

    // first time to express interest for this frame
    if ( getState() == StateFree )
    {
        state_ = StateNew;
        requestTimeUsec_ = segment->getRequestTimeUsec();
        prefix_ = name;
        frameNumber_ = frameNumber;
    }

    nSegmentPending ++;
}

void
FrameBuffer::Slot::appendData ( const ndn::Data &data )
{
    // get name component info
    Name name = data.getName();
    int frmNumber, segNumber;
    Namespacer::getSegmentationNumbers(name,frmNumber,segNumber);
    PrefixMetaInfo prefixMetaInfo;
    Namespacer::getPrefixMetaInfo(name,prefixMetaInfo);

    // set the Segment object
    boost::shared_ptr<Segment> segment;
    segment = prepareSegment( segNumber );    // get the segment by segment number
    SegmentData segmentData;
    SegmentData::segmentDataFromRaw(data.getContent().size(),
                                    data.getContent().buf(),
                                    segmentData);
    segment->dataArrived( *(segmentData.getMetaData()) );
    // append segment data block to Slot::slotData_ and update segment parameter
    segment->setDataPtr(addData( segmentData, segNumber ));
    segment->setPayloadSize(data.getContent().size());

    if( segment->getState() == Segment::StateMissing )
    {
        nSegmentMissed --;
    }

    segment->setSegmentNumber(segNumber);

    // update slot parameter
    State oldState = getState();
    if ( oldState == StateNew ) // first time to recieve segment in this slot
    {
        nSegmentTotal = prefixMetaInfo.totalSegmentNum_;
        // according to the first segment recieved,
        // we can know how many segment we should request
        updateSlot();
        setState( StateAssembling ); // change to next state (StateAssembling)
    }

    if( nSegmentReady == 1 )
        firstSegmentTimeUsec_ = segment->getArrivalTimeUsec();
    if( nSegmentReady == nSegmentTotal )
        readyTimeUsec_ = segment->getArrivalTimeUsec();
    nSegmentPending --;
    nSegmentReady ++;

    if( nSegmentReady == nSegmentTotal )
    {
        setState(Segment::StateFetched);
    }
}

void
FrameBuffer::Slot::markMissed(const ndn::Interest &interest)
{
    Name name = interest.getName();
    FrameNumber frameNo;
    SegmentNumber segNo;
    Namespacer::getSegmentationNumbers(name, frameNo, segNo );
    boost::shared_ptr<Segment> segment;
    segment = getSegment(segNo);
    if( segment.get() && segment->getState() == Segment::StatePending )
    {
        segment->markMissed();
        nSegmentMissed ++;
        nSegmentPending --;
    }
}

void
FrameBuffer::Slot::discard()
{
    reset();
}

// getter and setter
//************************************************
void
FrameBuffer::Slot::getMissedSegments(std::vector<SegmentNumber>& missedSegments)
{
    std::map<SegmentNumber, boost::shared_ptr<Segment>>::iterator iter;
    boost::shared_ptr<Segment> segment;
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
    readyTimeUsec_ = -1;

    nSegmentMissed = -1;
    nSegmentTotal = -1;
    nSegmentPending = -1;
    nSegmentReady = -1;

    assembledSize_ = 0;
    payloadSize_ = 0;
    //memset( slotData_,0, allocatedSize_);

    // clear all active segments if any
    std::map<SegmentNumber, boost::shared_ptr<Segment> >::iterator iter;
    for( iter = activeSegments_.begin(); iter != activeSegments_.end(); iter++ )
    {
        iter->second->discard();
        freeSegments_.push_back(iter->second);
        activeSegments_.erase(iter);
    }
    activeSegments_.clear();
}

/**
  * @brief pick a free segment from freeSegments_ and delete it from Slot::freeSegments_.
  * @param
  */
boost::shared_ptr<FrameBuffer::Slot::Segment>
FrameBuffer::Slot::pickFreeSegment()
{
    boost::shared_ptr<Segment> freeSegment;
    if( freeSegments_.size() )
    {
        freeSegment = freeSegments_.at(freeSegments_.size()-1);
        freeSegments_.pop_back();
    }
    else
    {
        freeSegment.reset(new Segment());
        //freeSegments_.push_back(freeSegment);
    }
    return freeSegment;
}

/**
  * @brief prepare a segment with segment number,
  *         this segment must in the Slot::activeSegments_,
  *         if its not in activeSegments_, then pick a free
  *         segment from freeSegments_ and set the segment number to segNo.
  * @param segNo, segment number
  */
boost::shared_ptr<FrameBuffer::Slot::Segment>
FrameBuffer::Slot::prepareSegment(SegmentNumber segNo)
{
    boost::shared_ptr<Segment> freeSegment;
    std::map<SegmentNumber, boost::shared_ptr<Segment> >::iterator iter;

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
    }
    return freeSegment;
}

/**
  * @brief get a segment by segNo from active segments.
  * @param segNo
  */
boost::shared_ptr<FrameBuffer::Slot::Segment>
FrameBuffer::Slot::getSegment(SegmentNumber segNo)
{
    boost::shared_ptr<Segment> segment;
    std::map<SegmentNumber, boost::shared_ptr<Segment> >::iterator iter;

    iter = activeSegments_.find(segNo);
    // segment exist in activeSegments_
    if( iter != activeSegments_.end() )
    {
        segment = iter->second;
    }
    return segment;
}

unsigned char*
FrameBuffer::Slot::addData( SegmentData segmentData, SegmentNumber segNo )
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
    memcpy( slotData_+segIdx, segmentData.getData(), segmentData.size() );
    return slotData_+segIdx;
}

void
FrameBuffer::Slot::updateSlot()
{
    if( nSegmentTotal == nSegmentPending )
        return;
    // we have express too much interest
    if( nSegmentTotal > nSegmentPending )
    {
        std::map<SegmentNumber, boost::shared_ptr<Segment>>::iterator it;
        boost::shared_ptr<Segment> segment;

        for ( it = activeSegments_.begin(); it != activeSegments_.end(); it++ )
        {
            segment = it->second;
            // the segment do not exist but we have express Interest
            // and create segment object for it, so it should be deleted
            if( segment->getSegmentNumber() > nSegmentTotal )
            {
                if( segment->getState() == Segment::StateMissing )
                    nSegmentMissed --;
                if( segment->getState() == Segment::StatePending )
                    nSegmentPending --;
                freeActiveSegment(it);
            }
        }
    }
    // segments number of this frame is more than we have expressed
    else
    {
        SegmentNumber iter;
        for ( iter = nSegmentPending; iter < nSegmentTotal; iter++ )
        {
            boost::shared_ptr<Segment> segment(new Segment());
            segment->setSegmentNumber(iter);
            segment->markMissed();
            segment-setState(Segment::StateMissing);
            nSegmentMissed++;
        }
    }
}

void
FrameBuffer::Slot::freeActiveSegment( SegmentNumber segNo )
{
    boost::shared_ptr<Segment> segment;
    std::map<SegmentNumber, boost::shared_ptr<Segment>>::iterator iter;
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
        (std::map<SegmentNumber, boost::shared_ptr<Segment>>::iterator iter )
{
    boost::shared_ptr<Segment> segment;
    segment = iter->second;
    segment->discard();
    freeSegments_.push_back(segment);
    activeSegments_.erase(iter);
}



//*****************************************************
//  FrameBuffer
//*****************************************************

int
FrameBuffer::init()
{
    reset();
    initialize();
}

void FrameBuffer::reset()
{
    lock_guard<recursive_mutex> scopedLock(syncMutex_);

    state_ = Invalid;
    playbackNo_ = -1;
    playbackSlot_.reset();
    callback_ = NULL;

    std::map<Name, boost::shared_ptr<Slot>>::iterator iter;
    boost::shared_ptr<Slot> slot;
    for ( iter = activeSlots_.begin(); iter != activeSlots_.end(); iter++ )
    {
        slot = iter->second;
        slot->reset();
        freeSlots_.push_back(slot);
    }
    activeSlots_.clear();
}

void
FrameBuffer::initialize( int slotNum )
{
    while( freeSlots_.size() < slotNum )
    {
        boost::shared_ptr<Slot> slot(new Slot);
        freeSlots_.push_back(slot);
    }

    state_ = Valid;
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

    boost::shared_ptr<Slot> slot = getSlot(name,false);
    slot->appendData(*(data.get()));

    checkMissed();
}

void
FrameBuffer::interestTimeout(const ndn::Interest &interest)
{
    Name name = interest.getName();
    int p = Namespacer::findComponent(name,NameComponents::NameComponentStreamFrameVideo);
    if (p<0)
        p = Namespacer::findComponent(name,NameComponents::NameComponentStreamFrameAudio);
    boost::shared_ptr<Slot> slot;
    slot = getSlot(name.getSubName(0,p+2),false);
    slot->markMissed(interest);
}

void
FrameBuffer::checkMissed()
{
    std::vector<SegmentNumber> missedSegs;
    boost::shared_ptr<Slot> slot;
    std::map<Name, boost::shared_ptr<Slot>>::iterator iter;
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

boost::shared_ptr<FrameBuffer::Slot>
FrameBuffer::getSlot(const ndn::Name& prefix, bool remove)
{
    boost::shared_ptr<Slot> slot;
    std::map<Name, boost::shared_ptr<Slot>>::iterator it;
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
}
