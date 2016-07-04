/*
 * frame-buffer.cpp
 *
 *  Created on: Jun 15, 2016
 *      Author: xyh
 */

#include <iostream>

#include "utils.h"
#include "frame-buffer.h"


////////////////////////////////////////////////////////////////
///     FrameBuffer::Slot
////////////////////////////////////////////////////////////////

FrameBuffer::Slot::Slot()
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
        requestTimeUsec_ = NdnRtcUtils::microsecondTimestamp();

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
    arrivalTimeUsec_ = NdnRtcUtils::microsecondTimestamp();
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
FrameBuffer::pushSlot(boost::shared_ptr<Slot> slot)
{
    std::lock_guard<std::recursive_mutex> scopedLock(syncMutex_);

    if(activeSlots_count_ >= 50)
    {
        //cout << "activeSlots_count_ " << activeSlots_count_ << endl;
        return false;
    }
    playbackQueue_.push(slot);
    activeSlots_count_++;

    return true;
}


void
FrameBuffer::dataArrived(const ndn::ptr_lib::shared_ptr<Data>& data)
{
    FrameNumber frameNo = std::atoi(data->getName().get(1).toEscapedString().c_str());
    map<int, boost::shared_ptr<Slot> >::iterator iter;
    iter = activeSlots_.find(frameNo);

    if ( iter == activeSlots_.end() )
    {
        cout << "FrameBuffer::dataArrived Error " << endl;
        return;
    }

    setSlot(data, iter->second);
}


void
FrameBuffer::setSlot(const ndn::ptr_lib::shared_ptr<Data>& data, boost::shared_ptr<FrameBuffer::Slot> slot)
{
    FrameData gotFrame;
    memcpy(&gotFrame, data->getContent().buf(), sizeof(FrameData));  //copy frame header

    gotFrame.buf_ = (unsigned char*) malloc (gotFrame.header_.length_);
    memcpy( gotFrame.buf_, data->getContent().buf()+sizeof(FrameDataHeader), gotFrame.header_.length_);   //copy frame data

    /*
    for( int i = 0; i <34; i++ )
            printf("%2X ",data->getContent().buf()[i]);
    cout << endl;

    for( int i = 0; i <34; i++ )
            printf("%2X ",gotFrame.buf_[i]);
    cout << endl;
    */

    slot->lock();

    slot->setDataPtr(gotFrame.buf_);



    //slot->setNumber(std::atoi(data->getName().get(1).toEscapedString()));
    slot->setPayloadSize(gotFrame.header_.length_);
    //slot->setPrefix(data->getName());
    slot->dataArrived();

    slot->unlock();

    //playbackQueue_.push(slot);

}


boost::shared_ptr<FrameBuffer::Slot>
FrameBuffer::popSlot()
{
	std::lock_guard<std::recursive_mutex> scopedLock(syncMutex_);

    if ( playbackQueue_.empty() )
        return NULL;

    boost::shared_ptr<Slot>  tmp;
    tmp = playbackQueue_.top();
	//FrameBuffer::Slot*  tmp = priorityQueue_.front();

    if( tmp->getState() != FrameBuffer::Slot::StateFetched )
        return NULL;

    map<int, boost::shared_ptr<Slot> >::iterator iter;

    iter = activeSlots_.find(tmp->getNumber());
    if( iter != activeSlots_.end() )
        activeSlots_.erase(iter);

    playbackQueue_.pop();
    activeSlots_count_--;

	return tmp;
}
