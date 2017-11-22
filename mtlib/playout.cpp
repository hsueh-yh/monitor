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
#include "glogger.h"



using namespace std;

const int Playout::BufferCheckInterval = 1000;
std::string PLAYOUT_LOG = "playout.log";
//boost::asio::io_service ioservice;

//******************************************************************************

Playout::Playout(Consumer *consumer):
    //MtNdnComponent(ioservice),
    jitterTiming_(new JitterTiming()),
    isRunning_(false),
    consumer_(consumer),
    //consumerId_(consumer_->getId()),
    vec_data_(nullptr)
{
    setDescription(MtNdnUtils::formatString("Playout"
                                            /*, getDescription().c_str()*/));
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
    reset(initialAdjustment);
    
    MtNdnUtils::dispatchOnBackgroundThread([this](){
        processPlayout();
    });
    LogTraceC << "started" << endl;

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
        
        LogTraceC << "stopped" << endl;
    }
    else
        return -1;
    
    /*if (data_)
    {
        delete data_;
        data_ = nullptr;
    }*/
    if (vec_data_)
    {
        delete vec_data_;
        vec_data_ = nullptr;
    }
    
    return 0;
}

void
Playout::setLogger(ndnlog::new_api::Logger *logger)
{
    jitterTiming_->setLogger(logger);
    ILoggingObject::setLogger(logger);
}

void
Playout::setDescription(const std::string &desc)
{
    ILoggingObject::setDescription(desc);
    jitterTiming_->setDescription(MtNdnUtils::formatString("JitterTiming"/*, getDescription().c_str()*/));
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
            if( vec_data_ )
            {
                delete vec_data_;
                vec_data_ = nullptr;
            }
            
            PacketNumber playbackPacketNo, sequencePacketNo, pairedPacketNo;
            double assembledLevel = 0;
            bool isKey, packetValid = false;
            bool skipped = false, missed = false,
                    outOfOrder = false, noData = false, incomplete = false;
            
            //frameBuffer_->acquireSlot(&data_, packetNo, sequencePacketNo,
            //                          pairedPacketNo, isKey, assembledLevel);
            //LogTraceC << "acquire frame" << endl;
            //vector<uint8_t> vdata_;
            vec_data_ = new vector<uint8_t>();
            int64_t captureTimeStampMs = -1;
            frameBuffer_->acquireFrame( *vec_data_,
                                        captureTimeStampMs,
                                        playbackPacketNo,
                                        sequencePacketNo,
                                        pairedPacketNo,
                                        isKey,
                                        assembledLevel);

            uint8_t *data_ = vec_data_->data();
            unsigned int dataSize = vec_data_->size();

            /*
             * LogTraceC
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
            if (playbackPacket(now, *vec_data_, playbackPacketNo, sequencePacketNo,
                               pairedPacketNo, isKey, assembledLevel))
            {
                packetValid = true;
            }

            if (data_)
            {
                updatePlaybackAdjustment(captureTimeStampMs);
                //LogTraceC << getDescription() << " latest" << std::endl;
                lastPacketTs_ = (captureTimeStampMs != -1 ? captureTimeStampMs : 0);
            }
            
            //******************************************************************
            // get playout time (delay) for the rendered frame
            int cachesize = 0;


            int playbackDelayMs = frameBuffer_->releaseAcquiredFrame(isInferredPlayback_);


            LogTraceC << "NextPlaybackDelay " << playbackDelayMs
                      << (isInferredPlayback_ ? ", inferred" : ", NOT inferred")
                      << std::endl<< std::endl;

            int adjustment = playbackDelayAdjustment(playbackDelayMs, cachesize);
            
            if (playbackDelayMs < 0)
            {
                // should never happen
                LogWarnC << " playback delay below zero: " << playbackDelayMs << endl;
                playbackDelayMs = 0;
            }

            playbackDelayMs += adjustment;
            if( playbackDelayMs  < 0 )
                playbackDelayMs = 0;
            assert(playbackDelayMs >= 0);

            playoutMutex_.unlock();
            
            if( noData )
                playbackDelayMs = 30;
            if (isRunning_)
            {
                // setup and run playout timer for calculated playout interval
                jitterTiming_->updatePlayoutTime(playbackDelayMs, sequencePacketNo);
                jitterTiming_->run(bind(&Playout::processPlayout, this));
            }
        }
    }
    else
        playoutMutex_.unlock();
    
    return isRunning_;
}

void
Playout::updatePlaybackAdjustment(int64_t capTsMs)
{
    // check if previous frame playout time was inferred
    // if so - calculate adjustment
    if (lastPacketTs_ > 0 && isInferredPlayback_)
    {
        int realPlayback = capTsMs-lastPacketTs_;
        playbackAdjustment_ += (realPlayback-inferredDelay_);
        //playbackAdjustment_ += 0;
        inferredDelay_ = 0;
    }
}

int
Playout::playbackDelayAdjustment(int playbackDelay, int cachesize )
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
    
    LogTraceC << "updated total adj is " << playbackAdjustment_ << std::endl;
    
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
    //LogTraceC << "CheckBuffer " << std::endl;
    int64_t timestamp = MtNdnUtils::millisecondTimestamp();
    if (timestamp - bufferCheckTs_ > BufferCheckInterval)
    {
        bufferCheckTs_ = timestamp;
        
        // keeping buffer level at the target size
        //unsigned int targetBufferSize = consumer_->getBufferEstimator()->getTargetSize();
        int targetBufferSize = 90; //about 5 frames
        int playableDuration = consumer_->getFrameBuffer()->getPlayableBufferDuration();
        int adjustment = targetBufferSize - playableDuration;
        
        LogTraceC << "buffer size " << playableDuration << std::endl;
        
        if (abs(adjustment) > 30 && adjustment < 0)
        {
            LogTraceC << "bf adj. "
            << abs(adjustment) << "ms excess" << std::endl;
            
            playbackAdjustment_ += adjustment;
        }
    }
}

