//
//  jitter-timing.cpp
//  mtndn
//
//  Created by Peter Gusev on 5/8/14.
//  Copyright 2013-2015 Regents of the University of California
//

#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "jitter-timing.h"
#include "mtndn-utils.h"
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
    VLOG(LOG_TRACE) << "[JitterTiming] flushed" << std::endl;
}
void JitterTiming::stop()
{
    stopJob();
    VLOG(LOG_TRACE) << "[JitterTiming] stopped" << std::endl;
}

int64_t JitterTiming::startFramePlayout()
{
    int64_t processingStart = MtNdnUtils::microsecondTimestamp();
    VLOG(LOG_TRACE) << "[JitterTiming] proc start " << processingStart << std::endl;
    
    if (prevPlayoutTsUsec_ == 0)
    {
        prevPlayoutTsUsec_ = processingStart;
    }
    else
    { // calculate processing delay from the previous iteration
        int64_t prevIterationProcTimeUsec = processingStart - prevPlayoutTsUsec_;
        
        VLOG(LOG_TRACE) << "[JitterTiming] prev iter full time " << prevIterationProcTimeUsec << std::endl;

        // substract frame playout delay
        if (prevIterationProcTimeUsec >= framePlayoutTimeMs_*1000)
            prevIterationProcTimeUsec -= framePlayoutTimeMs_*1000;
        else // should not occur!
        {
            /*
            VLOG(LOG_TRACE) << "[JitterTiming] assertion failed: "
            << "prevIterationProcTimeUsec ("
            << prevIterationProcTimeUsec << ") < framePlayoutTimeMs_*1000"
            << "(" << framePlayoutTimeMs_*1000 << ")" << endl;
            */
            assert(0);
        }
        
        VLOG(LOG_TRACE) << "[JitterTiming] prev iter proc time " << prevIterationProcTimeUsec << std::endl;
        
        // add this time to the average processing time
        processingTimeUsec_ += prevIterationProcTimeUsec;

        VLOG(LOG_TRACE) << "[JitterTiming] total proc time " << processingTimeUsec_ << std::endl;
        
        prevPlayoutTsUsec_ = processingStart;
    }
    
    return prevPlayoutTsUsec_;
}

void JitterTiming::updatePlayoutTime(int framePlayoutTime, PacketNumber packetNo)
{
    VLOG(LOG_TRACE) << "[JitterTiming] packet " << packetNo << " playout time " << framePlayoutTime << std::endl;
    
    int playoutTimeUsec = framePlayoutTime*1000;
    if (playoutTimeUsec < 0) playoutTimeUsec = 0;
    
    if (processingTimeUsec_ >= 1000)
    {
        VLOG(LOG_TRACE) << "[JitterTiming] absorb proc time " << processingTimeUsec_ << std::endl;
        
        int processingUsec = (processingTimeUsec_/1000)*1000;
        
        VLOG(LOG_TRACE) << "[JitterTiming] proc absorb part " << processingUsec << std::endl;
        
        if (processingUsec > playoutTimeUsec)
        {
            VLOG(LOG_TRACE) << "[JitterTiming] skip frame. proc " << processingUsec
            << " playout " << playoutTimeUsec << std::endl;
            
            processingUsec = playoutTimeUsec;
            playoutTimeUsec = 0;
        }
        else
            playoutTimeUsec -= processingUsec;
        
        processingTimeUsec_ = processingTimeUsec_ - processingUsec;
        VLOG(LOG_TRACE) << "[JitterTiming] playout usec " << playoutTimeUsec
                  << "us, total proc " << processingTimeUsec_ << "us" << std::endl;
    }
    
    framePlayoutTimeMs_ = playoutTimeUsec/1000;
}

void JitterTiming::run(boost::function<void()> callback)
{
    assert(framePlayoutTimeMs_ >= 0);
    //int64_t now = MtNdnUtils::microsecondTimestamp();
    VLOG(LOG_TRACE) << "[JitterTiming] timer wait " << framePlayoutTimeMs_ << "ms ... "
              << MtNdnUtils::microsecondTimestamp() << std::endl;
    //std::cout << "jt " << now << std::endl;
    //LOG(WARNING) << "[JT] " << MtNdnUtils::microsecondTimestamp();
    scheduleJob(2, framePlayoutTimeMs_*1000, [this, callback]()->bool{
        callback();
        return false;
    });
}

//******************************************************************************
void JitterTiming::resetData()
{
    framePlayoutTimeMs_ = 0;
    processingTimeUsec_ = 0;
    prevPlayoutTsUsec_ = 0;
}
