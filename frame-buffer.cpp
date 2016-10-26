/*
 * frame-buffer.cpp
 *
 *  Created on: Jun 15, 2016
 *      Author: xyh
 */

#include <iostream>

#include "utils.h"
#include "frame-buffer.h"
#include "statistics.hpp"
#include "logger.hpp"
#include "namespacer.h"


////////////////////////////////////////////////////////////////
///     FrameBuffer::Slot
////////////////////////////////////////////////////////////////

FrameBuffer::Slot::Slot():
    statistic(Statistics::getInstance())
{
    resetData();
}


FrameBuffer::Slot::~Slot()
{}

void
FrameBuffer::Slot::discard()
{
    resetData();
}

void
FrameBuffer::Slot::resetData()
{
    state_ = FrameBuffer::Slot::StateNotUsed;
    requestTimeUsec_ = -1;
    arrivalTimeUsec_ = -1;
    //reqCounter_ = 0;
    frameNumber_ = -1;
    payloadSize_ = -1;
    prefix_ = Name();
}

void
FrameBuffer::Slot::interestIssued()
{

    state_ = StatePending;

    if (requestTimeUsec_ <= 0)
        requestTimeUsec_ = NdnUtils::microsecondTimestamp();

    //reqCounter_++;
}

void
FrameBuffer::Slot::markMissed()
{
    state_ = StateMissing;
}

void
FrameBuffer::Slot::dataArrived ()
{
    state_ = StateFetched;
    arrivalTimeUsec_ = NdnUtils::microsecondTimestamp();
    uint64_t delay = arrivalTimeUsec_-requestTimeUsec_;
    statistic->addData(delay);
    LOG(INFO) << "[FrameBuffer] Recieve Data " << prefix_.to_uri() <<  " Delay " << delay/1000
              << "ms ( average: " << statistic->getDelay()/1000 << "ms )" << endl;
}



////////////////////////////////////////////////////////////////
///     FrameBuffer::PlaybackQueue
////////////////////////////////////////////////////////////////
/*
FrameBuffer::PlaybackQueue::PlaybackQueue(double playbackRate):
    playbackRate_(playbackRate),
    lastFrameDuration_(ceil(1000./playbackRate)),
    comparator_(FrameBuffer::Slot::PlaybackComparator(false))
{}



int64_t
FrameBuffer::PlaybackQueue::getPlaybackDuration(bool estimate)
{
    int64_t playbackDurationMs = 0;

    if (this->size() >= 1)
    {
        PlaybackQueueBase::iterator it = this->begin();
        Slot* slot1 = *it;

        while (++it != this->end())
        {
            Slot* slot2 = *it;

            // if header metadata is available - we have frame timestamp
            // and frames are consequent - we have real playback time
            if (slot1->getConsistencyState()&Slot::HeaderMeta &&
                slot2->getConsistencyState()&Slot::HeaderMeta &&
                slot1->getPlaybackNumber() + 1 == slot2->getPlaybackNumber())
            {
                lastFrameDuration_ = (slot2->getProducerTimestamp() - slot1->getProducerTimestamp());
                // duration can be 0 if we got RTCP and RTP packets
                assert(lastFrameDuration_ >= 0);
                playbackDurationMs += lastFrameDuration_;
            }
            else
            {
                // otherwise - infer playback duration from the producer rate
                playbackDurationMs += (estimate)?getInferredFrameDuration():0;
            }

            slot1 = slot2;
        }
    }

    return playbackDurationMs;
}

void
FrameBuffer::PlaybackQueue::updatePlaybackDeadlines()
{
    sort();
    int64_t playbackDeadlineMs = 0;

    if (this->size() >= 1)
    {
        PlaybackQueueBase::iterator it = this->begin();
        Slot* slot1 = *it;

        while (++it != this->end())
        {
            slot1->setPlaybackDeadline(playbackDeadlineMs);
            Slot* slot2 = *it;

            // if header metadata is available - we have frame timestamp
            if (slot1->getConsistencyState()&Slot::HeaderMeta &&
                slot2->getConsistencyState()&Slot::HeaderMeta)
            {
                playbackDeadlineMs += (slot2->getProducerTimestamp() - slot1->getProducerTimestamp());
            }
            else
            {
                // otherwise - infer playback duration from the producer rate
                playbackDeadlineMs += getInferredFrameDuration();
            }

            slot1 = slot2;
        }

        // set deadline for the last frame
        slot1->setPlaybackDeadline(playbackDeadlineMs);
    }
}

void
FrameBuffer::PlaybackQueue::pushSlot
(FrameBuffer::Slot* const slot)
{
    LogTraceC << "▼push[" << slot->dump() << "]" << std::endl;
    this->push_back(slot);

    updatePlaybackDeadlines();
    dumpQueue();
}

FrameBuffer::Slot*
FrameBuffer::PlaybackQueue::peekSlot()
{
    return (0 != this->size()) ? *(this->begin()) : nullptr;
}

void
FrameBuffer::PlaybackQueue::popSlot()
{
    FrameBuffer::Slot* slot;

    if (0 != this->size())
    {
        slot = *(this->begin());
        LogTraceC << "▲pop [" << slot->dump() << "]" << std::endl;

        this->erase(this->begin());
        updatePlaybackDeadlines();

        dumpQueue();
    }
}

void
FrameBuffer::PlaybackQueue::updatePlaybackRate(double playbackRate)
{
    playbackRate_ = playbackRate;
}

void
FrameBuffer::PlaybackQueue::clear()
{
    PlaybackQueueBase::clear();
}

void
FrameBuffer::PlaybackQueue::sort()
{
    std::sort(this->begin(), this->end(), comparator_);
}

void
FrameBuffer::PlaybackQueue::dumpQueue()
{
    if (this->logger_->getLogLevel() != NdnLoggerDetailLevelAll)
        return;

    PlaybackQueueBase::iterator it;
    int i = 0;

    for (it = this->begin(); it != this->end(); ++it)
    {
        LogTraceC
        << "[" << std::setw(3) << i++ << ": "
        << (*it)->dump() << "]" << std::endl;
    }
}

std::string
FrameBuffer::PlaybackQueue::dumpShort()
{
    std::stringstream ss;
    PlaybackQueueBase::iterator it;
    int nSkipped = 0;

    ss << "[" ;
    for (it = this->begin(); it != this->end(); ++it)
    {
        if (it+2 == this->end())
            ss << " +" << nSkipped << " ";

        if (it == this->begin() ||
            it+1 == this->end() || it+2 == this->end() ||
            (*it)->getNamespace() == Slot::Key)
        {
            if (nSkipped != 0 && it < this->end()-2)
            {
                ss << " +" << nSkipped << " ";
                nSkipped = 0;
            }

            ss << (*it)->getSequentialNumber() << "(";

            if (it+1 == this->end() ||
                it+2 == this->end() ||
                (*it)->getNamespace() == Slot::Key)
                ss << (*it)->getPlaybackDeadline() << "|";

            ss << round((*it)->getAssembledLevel()*100)/100 << ")";
        }
        else
            nSkipped++;
    }
    ss << "]";

    return ss.str();
}

*/

////////////////////////////////////////////////////////////////
///     FrameBuffer
////////////////////////////////////////////////////////////////

bool
FrameBuffer::interestIssued( ndn::Interest& interest )
{
    ptr_lib::lock_guard<ptr_lib::recursive_mutex> scopedLock(syncMutex_);

    ptr_lib::shared_ptr<FrameBuffer::Slot> slot;
    ndn::Name prefix = interest.getName();

    slot.reset(new FrameBuffer::Slot());
    slot->lock();
    slot->setPrefix(prefix);
    PacketNumber pktNo;
    Namespacer::getFrameNumber(prefix,pktNo);
    slot->setNumber(pktNo);
    slot->interestIssued();
    slot->unlock();

//    if(activeSlots_count_ >= 50)
//    {
//        //cout << "activeSlots_count_ " << activeSlots_count_ << endl;
//        return false;
//    }
    //playbackQueue_.push(slot);
    activeSlots_[pktNo] = slot;
    //readySlots_count_++;
    LOG(INFO) << "[FrameBuffer] Interest Issued " << slot->getPrefix().to_uri()
                 << " ( Total:" << activeSlots_.size()
                 << " Ready: " <<readySlots_count_ << " )"<< endl;

    return true;
}

void
FrameBuffer::dataArrived(const ndn::ptr_lib::shared_ptr<Data>& data)
{
    FrameNumber frameNo;
    Namespacer::getFrameNumber(data->getName(),frameNo);
    map<unsigned int, ptr_lib::shared_ptr<Slot> >::iterator iter;
    iter = activeSlots_.find(frameNo);

    if ( iter == activeSlots_.end() )
    {
        cout << "FrameBuffer::dataArrived Error " << endl;
        return;
    }

    readySlots_count_++;

    // set slot
    ptr_lib::shared_ptr<FrameBuffer::Slot> slot = iter->second;
    slot->lock();

    slot->setNdnDataPtr(data);
    slot->setDataPtr(data->getContent().buf());
    slot->setNumber(frameNo);
    slot->setPayloadSize(data->getContent().size());
    slot->dataArrived();

//    cout << "set Slot: No=" << slot->getFrameNo()
//         << " name=" << slot->getPrefix().toUri()
//         << " payload=" << slot->getPayloadSize()
//         << " addr=" << (void*)slot->getDataPtr();
//    NdnUtils::printMem("set Slot", slot->getDataPtr(),20);

    slot->unlock();
    //setSlot(data, iter->second);
}

void
FrameBuffer::dataMissed(const ptr_lib::shared_ptr<const Interest>& interest )
{
    int componentCount = interest->getName().getComponentCount();
    FrameNumber frameNo = std::atoi(interest->getName().get(componentCount-1).toEscapedString().c_str());
    map<unsigned int, ptr_lib::shared_ptr<Slot> >::iterator iter;
    iter = activeSlots_.find(frameNo);

    if ( iter == activeSlots_.end() )
    {
        cout << "FrameBuffer::dataMissed Error " << endl;
        return;
    }
    ptr_lib::shared_ptr<Slot> slot = iter->second;
    slot->markMissed();

    LOG(INFO) << "[FrameBuffer] miss " << interest->getName().to_uri()
              << " ( Total:" << activeSlots_.size()
              << " Ready: " <<readySlots_count_ << " )"<< endl;
}

ptr_lib::shared_ptr<FrameBuffer::Slot>
FrameBuffer::acquireData()
{
    std::lock_guard<std::recursive_mutex> scopedLock(syncMutex_);

    if ( readySlots_count_ < 5 )
    {
        LOG(INFO) << "[Player] empty"
                  << " ( Total:" << activeSlots_.size()
                  << " Ready: " <<readySlots_count_ << " )"<< endl;
        return NULL;
    }

    ptr_lib::shared_ptr<Slot>  slot;
    //slot = playbackQueue_.top();
    map<unsigned int, ptr_lib::shared_ptr<Slot> >::iterator iter;
    iter = activeSlots_.begin();
    slot = iter->second;
    //FrameBuffer::Slot*  tmp = priorityQueue_.front();

    if( slot->getState() != FrameBuffer::Slot::StateFetched )
    {
        if( slot->getFrameNo() < 3 )
            return NULL;
       // LOG(WARNING) << "[FrameBuffer] Skip " << slot->getPrefix()
        //             << " ( remain " << activeSlots_count_ << " )"<< endl;
        //playbackQueue_.pop();
        //activeSlots_count_--;
        //return NULL;
    }

    activeSlots_.erase(iter);
    readySlots_count_--;

    LOG(INFO) << "[FrameBuffer] pop " << slot->getPrefix()
              << " ( Total:" << activeSlots_.size()
              << " Ready: " <<readySlots_count_ << " )"<< endl;

//    cout << "get Slot: No=" << slot->getFrameNo()
//         << " name=" << slot->getPrefix().toUri()
//         << " payload=" << slot->getPayloadSize()
//         << " addr=" << (void*)slot->getDataPtr();
//    NdnUtils::printMem("get Slot", slot->getDataPtr(),20);
    return slot;
}

unsigned int
FrameBuffer::acquireData( unsigned char* buf )
{
    std::lock_guard<std::recursive_mutex> scopedLock(syncMutex_);

    if ( readySlots_count_ < 1 )
    {
        LOG(INFO) << "[Player] empty"
                  << " ( Total:" << activeSlots_.size()
                  << " Ready: " <<readySlots_count_ << " )"<< endl;
        return 0;
    }
    if ( readySlots_count_ < 5 )
    {
        return 0;
    }

    ptr_lib::shared_ptr<Slot>  slot;
    map<unsigned int, ptr_lib::shared_ptr<Slot> >::iterator iter;

    unsigned char *slotbuf;
    int nalCounter = 0;
    unsigned int bufsize = 0;

    while(nalCounter <= 1)
    {
        iter = activeSlots_.begin();
        slot = iter->second;
        // this data is not fetched
        if( slot->getState() != FrameBuffer::Slot::StateFetched )
        {
            break;
        }
        slotbuf = slot->getDataPtr();
        if( 0 == slotbuf[0] && 0 == slotbuf[1] && 0 == slotbuf[2] && 1 == slotbuf[3])
            ++nalCounter;
        if( nalCounter > 1 )
            break;
        activeSlots_.erase(iter);
        readySlots_count_--;

        //NdnUtils::printMem("pop", slotbuf, 20);
        memcpy(buf+bufsize,slot->getDataPtr(),slot->getPayloadSize());
        bufsize+=slot->getPayloadSize();

        LOG(INFO) << "[FrameBuffer] pop " << slot->getPrefix()
                  << " ( Total:" << activeSlots_.size()
                  << " Ready: " <<readySlots_count_ << " )"<< endl;

    }

    return bufsize;
}

void
FrameBuffer::setSlot(const ndn::ptr_lib::shared_ptr<Data>& data, ptr_lib::shared_ptr<FrameBuffer::Slot> slot)
{
    /*
    for( int i = 0; i <34; i++ )
            printf("%2X ",data->getContent().buf()[i]);
    cout << endl;

    for( int i = 0; i <34; i++ )
            printf("%2X ",gotFrame.buf_[i]);
    cout << endl;
    */



}

