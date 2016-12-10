//
//  jitter-timing.cpp
//  ndnrtc
//
//  Created by Peter Gusev on 5/8/14.
//  Copyright 2013-2015 Regents of the University of California
//

#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "jitter-timing.h"
#include "utils.h"
#include "logger.h"

using namespace std;

//******************************************************************************
#pragma mark - construction/destruction
JitterTiming::JitterTiming()
{
    resetData();
}

JitterTiming::~JitterTiming()
{
}

//******************************************************************************
#pragma mark - public
void JitterTiming::flush()
{
    resetData();
    LOG(INFO) << "[JitterTiming] flushed" << std::endl;
}
void JitterTiming::stop()
{
    stopJob();
    LOG(INFO) << "[JitterTiming] stopped" << std::endl;
}

int64_t JitterTiming::startFramePlayout()
{
    int64_t processingStart = NdnUtils::microsecondTimestamp();
    //LOG(INFO) << "[JitterTiming] [ proc start " << processingStart << endl;
    
    if (playoutTimestampUsec_ == 0)
    {
        playoutTimestampUsec_ = processingStart;
    }
    else
    { // calculate processing delay from the previous iteration
        int64_t prevIterationProcTimeUsec = processingStart -
        playoutTimestampUsec_;
        
        //LOG(INFO) << "[JitterTiming] . prev iter full time " << prevIterationProcTimeUsec << endl;
        
        // substract frame playout delay
        if (prevIterationProcTimeUsec >= framePlayoutTimeMs_*1000)
            prevIterationProcTimeUsec -= framePlayoutTimeMs_*1000;
        else
            // should not occur!
        {
            /*
            LOG(INFO) << "[JitterTiming] assertion failed: "
            << "prevIterationProcTimeUsec ("
            << prevIterationProcTimeUsec << ") < framePlayoutTimeMs_*1000"
            << "(" << framePlayoutTimeMs_*1000 << ")" << endl;
            */
            assert(0);
        }
        
        //LOG(INFO) << "[JitterTiming] . prev iter proc time " << prevIterationProcTimeUsec << endl;
        
        // add this time to the average processing time
        processingTimeUsec_ += prevIterationProcTimeUsec;
        //LOG(INFO) << "[JitterTiming] . total proc time " << processingTimeUsec_ << endl;
        
        playoutTimestampUsec_ = processingStart;
    }
    
    return playoutTimestampUsec_;
}

void JitterTiming::updatePlayoutTime(int framePlayoutTime, PacketNumber packetNo)
{
    //LOG(INFO) << "[JitterTiming] . packet " << packetNo << " playout time " << framePlayoutTime << endl;
    
    int playoutTimeUsec = framePlayoutTime*1000;
    if (playoutTimeUsec < 0) playoutTimeUsec = 0;
    
    if (processingTimeUsec_ >= 1000)
    {
        //LOG(INFO) << "[JitterTiming] . absorb proc time " << processingTimeUsec_ << endl;
        
        int processingUsec = (processingTimeUsec_/1000)*1000;
        
        //LOG(INFO) << "[JitterTiming] . proc absorb part " << processingUsec << endl;
        
        if (processingUsec > playoutTimeUsec)
        {
            //LOG(INFO) << "[JitterTiming] . skip frame. proc " << processingUsec
            //<< " playout " << playoutTimeUsec << endl;
            
            processingUsec = playoutTimeUsec;
            playoutTimeUsec = 0;
        }
        else
            playoutTimeUsec -= processingUsec;
        
        processingTimeUsec_ = processingTimeUsec_ - processingUsec;
        //LOG(INFO) << "[JitterTiming]. playout usec " << playoutTimeUsec
        //<< " total proc " << processingTimeUsec_ << endl;
    }
    
    framePlayoutTimeMs_ = playoutTimeUsec/1000;
}

void JitterTiming::run(boost::function<void()> callback)
{
    assert(framePlayoutTimeMs_ >= 0);
    
    LOG(INFO) << "[JitterTiming] . timer wait " << framePlayoutTimeMs_ << endl;
    
    scheduleJob(framePlayoutTimeMs_*1000, [this, callback]()->bool{
        callback();
        return false;
    });
}

//******************************************************************************
void JitterTiming::resetData()
{
    framePlayoutTimeMs_ = 0;
    processingTimeUsec_ = 0;
    playoutTimestampUsec_ = 0;
}
