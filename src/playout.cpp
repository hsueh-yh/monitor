//
//  playout.cpp
//  mtndn
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//  Created: 3/19/14
//

#include "playout.h"
#include "mtndn-utils.h"
#include "jitter-timing.h"
#include "consumer.h"
#include "logger.h"

using namespace std;

const int Playout::BufferCheckInterval = 2000;

//boost::asio::io_service ioservice;

//******************************************************************************

Playout::Playout(Consumer *consumer):
    //MtNdnComponent(ioservice),
    jitterTiming_(new JitterTiming()),
    isRunning_(false),
    consumer_(consumer),
    //consumerId_(consumer_->getId()),
    vdata_(nullptr)
{
    jitterTiming_->flush();
    
    if (consumer_)
    {
        frameBuffer_ = consumer_->getFrameBuffer();
    }
}

Playout::~Playout()
{
    if (isRunning_)
        stop();
}

//******************************************************************************
//public
int
Playout::init(void *frameConsumer)
{
    encodedFrameConsumer_ = frameConsumer;
    
    return 0;
}

int
Playout::start(int initialAdjustment)
{
    {
        ptr_lib::lock_guard<ptr_lib::mutex> scopeLock(playoutMutex_);
        
        jitterTiming_->flush(); 
        
        isRunning_ = true;
        isInferredPlayback_ = false;
        lastPacketTs_ = 0;
        inferredDelay_ = 0;
        playbackAdjustment_ = initialAdjustment;
        bufferCheckTs_ = MtNdnUtils::millisecondTimestamp();
    }
    
    MtNdnUtils::dispatchOnBackgroundThread([this](){
        processPlayout();
    });
    
    VLOG(LOG_INFO) << "[Playout] started" << endl;
    return 0;
}

int
Playout::stop()
{
    ptr_lib::lock_guard<ptr_lib::mutex> scopeLock(playoutMutex_);
    
    if (isRunning_)
    {
        isRunning_ = false;
        jitterTiming_->stop();
        
        VLOG(LOG_INFO) << "[Playout] stopped" << endl;
    }
    else
        return -1;
    
    /*if (data_)
    {
        delete data_;
        data_ = nullptr;
    }*/
    if (vdata_)
    {
        delete vdata_;
        vdata_ = nullptr;
    }
    
    return 0;
}

//******************************************************************************
//private
bool
Playout::processPlayout()
{
    playoutMutex_.lock();
    
    if (isRunning_)
    {
        int64_t now = MtNdnUtils::millisecondTimestamp();
        
        if (frameBuffer_->getState() == FrameBuffer::Valid)
        {
            checkBuffer();
            jitterTiming_->startFramePlayout();
            
            // cleanup from previous iteration
            /*
            if (data_)
            {
                delete data_;
                data_ = nullptr;
            }*/
            if( vdata_ )
            {
                delete vdata_;
                vdata_ = nullptr;
            }
            
            PacketNumber packetNo, sequencePacketNo, pairedPacketNo;
            double assembledLevel = 0;
            bool isKey, packetValid = false;
            bool skipped = false, missed = false,
                    outOfOrder = false, noData = false, incomplete = false;
            
            //frameBuffer_->acquireSlot(&data_, packetNo, sequencePacketNo,
            //                          pairedPacketNo, isKey, assembledLevel);
            //VLOG(LOG_TRACE) << "acquire frame" << endl;
            //vector<uint8_t> vdata_;
            vdata_ = new vector<uint8_t>();
            int64_t ts;
            frameBuffer_->acquireFrame( *vdata_,
                                        ts,
                                        packetNo,
                                        sequencePacketNo,
                                        pairedPacketNo,
                                        isKey,
                                        assembledLevel);

            uint8_t *data_ = vdata_->data();
            unsigned int dataLength = vdata_->size();

            /*
             * VLOG(LOG_TRACE)
            << "Gotframe size: "
            << dataLength << " "
            << hex << (void*)data_ << dec << " "
            << packetNo << " "
            << sequencePacketNo << " "
            << pairedPacketNo << " "
            << isKey << " "
            << assembledLevel << " "
            << std::endl;
            */
            noData = (data_ == nullptr);
            incomplete = (assembledLevel < 1.);
            
            //******************************************************************
            // next call is overriden by specific playout mechanism - either
            // video or audio. the rest of the code is similar for both cases
            if (playbackPacket(now, data_, dataLength, packetNo, sequencePacketNo,
                               pairedPacketNo, isKey, assembledLevel))
            {
                packetValid = true;
            }

            if (data_)
            {
                updatePlaybackAdjustment(ts);
                //VLOG(LOG_TRACE) << "[Playout] latest" << std::endl;
                lastPacketTs_ = (ts != -1 ? ts : 0);
            }
            
            //******************************************************************
            // get playout time (delay) for the rendered frame
            int playbackDelay = frameBuffer_->releaseAcquiredFrame(isInferredPlayback_);
            VLOG(LOG_TRACE) << "nextPlaybackDelay " << playbackDelay
                      << (isInferredPlayback_ ? ", inferred" : ", NOT inferred")
                      << std::endl;

            int adjustment = playbackDelayAdjustment(playbackDelay);
            
            if (playbackDelay < 0)
            {
                // should never happen
                VLOG(LOG_WARN) << "[Playout] playback delay below zero: " << playbackDelay << endl;
                playbackDelay = 0;
            }

            playbackDelay += adjustment;
            assert(playbackDelay >= 0);

            playoutMutex_.unlock();
            
            if (isRunning_)
            {
                // setup and run playout timer for calculated playout interval
                jitterTiming_->updatePlayoutTime(playbackDelay, sequencePacketNo);
                jitterTiming_->run(bind(&Playout::processPlayout, this));
            }
        }
    }
    else
        playoutMutex_.unlock();
    
    return isRunning_;
}

void
Playout::updatePlaybackAdjustment(int64_t ts)
{
    // check if previous frame playout time was inferred
    // if so - calculate adjustment
    if (lastPacketTs_ > 0 && isInferredPlayback_)
    {
        int realPlayback = ts-lastPacketTs_;
        playbackAdjustment_ += (realPlayback-inferredDelay_);
        //playbackAdjustment_ += 0;
        inferredDelay_ = 0;
    }
}

int
Playout::playbackDelayAdjustment(int playbackDelay)
{
    int adjustment = 0;
    
    if (isInferredPlayback_)
        inferredDelay_ += playbackDelay;
    else
        inferredDelay_ = 0;
    
    if (playbackAdjustment_ < 0 && abs(playbackAdjustment_) > playbackDelay)
    {
        playbackAdjustment_ += playbackDelay;
        adjustment = -playbackDelay;
    }
    else
    {
        adjustment = playbackAdjustment_;
        playbackAdjustment_ = 0;
    }
    
    VLOG(LOG_TRACE) << "[Playout] updated total adj is " << playbackAdjustment_ << std::endl;
    
    return adjustment;
}

/*
int
Playout::avSyncAdjustment(int64_t nowTimestamp, int playbackDelay)
{
    int syncDriftAdjustment = 0;
    
    if (consumer_->getAvSynchronizer().get())
    {
        syncDriftAdjustment = consumer_->getAvSynchronizer()->synchronizePacket(lastPacketTs_, nowTimestamp, (Consumer*)consumer_);
        
        if (abs(syncDriftAdjustment) > playbackDelay &&syncDriftAdjustment < 0)
        {
            playbackAdjustment_ = syncDriftAdjustment+playbackDelay;
            syncDriftAdjustment = -playbackDelay;
        }
        if (syncDriftAdjustment > 0 &&
            syncDriftAdjustment > AudioVideoSynchronizer::MaxAllowableAvSyncAdjustment)
            syncDriftAdjustment = AudioVideoSynchronizer::MaxAllowableAvSyncAdjustment;
    }
    
    return syncDriftAdjustment;
}
*/

void
Playout::checkBuffer()
{
    VLOG(LOG_TRACE) << "[Playout] checkBuffer " << std::endl;
    int64_t timestamp = MtNdnUtils::millisecondTimestamp();
    if (timestamp - bufferCheckTs_ > BufferCheckInterval)
    {
        bufferCheckTs_ = timestamp;
        
        // keeping buffer level at the target size
        //unsigned int targetBufferSize = consumer_->getBufferEstimator()->getTargetSize();
        int targetBufferSize = 150; //about 5 frames
        int playableDuration = consumer_->getFrameBuffer()->getPlayableBufferDuration();
        int adjustment = targetBufferSize - playableDuration;
        
        VLOG(LOG_TRACE) << "[Playout] buffer size " << playableDuration << std::endl;
        
        if (abs(adjustment) > 30 && adjustment < 0)
        {
            VLOG(LOG_TRACE) << "bf adj. "
            << abs(adjustment) << " ms excess" << std::endl;
            
            playbackAdjustment_ += adjustment;
        }
    }
}

