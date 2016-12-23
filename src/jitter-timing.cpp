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
#include "glogger.h"

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
    LogTraceC << "flushed" << std::endl;
}
void JitterTiming::stop()
{
    stopJob();
    LogTraceC << "stopped" << std::endl;
}

int64_t JitterTiming::startFramePlayout()
{
    int64_t processingStart = MtNdnUtils::microsecondTimestamp();
    LogTraceC << "proc start " << processingStart;// << std::endl;
    
    if (prevPlayoutTsUsec_ == 0)
    {
        prevPlayoutTsUsec_ = processingStart;
    }
    else
    { // calculate processing delay from the previous iteration
        int64_t prevIterationProcTimeUsec = processingStart - prevPlayoutTsUsec_;
        
        LogTraceC << " prev iter full time " << prevIterationProcTimeUsec;// << std::endl;

        // substract frame playout delay
        if (prevIterationProcTimeUsec >= framePlayoutTimeMs_*1000)
            prevIterationProcTimeUsec -= framePlayoutTimeMs_*1000;
        else // should not occur!
        {
            /*
            LogTraceC << "[JitterTiming] assertion failed: "
            << "prevIterationProcTimeUsec ("
            << prevIterationProcTimeUsec << ") < framePlayoutTimeMs_*1000"
            << "(" << framePlayoutTimeMs_*1000 << ")" << endl;
            */
            logger_->flush();
            assert(0);
        }
        
        LogTraceC << " prev iter proc time " << prevIterationProcTimeUsec;// << std::endl;
        
        // add this time to the average processing time
        processingTimeUsec_ += prevIterationProcTimeUsec;

        LogTraceC << " total proc time " << processingTimeUsec_ << std::endl;
        
        prevPlayoutTsUsec_ = processingStart;
    }
    
    return prevPlayoutTsUsec_;
}

void JitterTiming::updatePlayoutTime(int framePlayoutTime, PacketNumber packetNo)
{
    LogTraceC << " packet " << packetNo << " playout time " << framePlayoutTime;// << std::endl;
    
    int playoutTimeUsec = framePlayoutTime*1000;
    if (playoutTimeUsec < 0) playoutTimeUsec = 0;
    
    if (processingTimeUsec_ >= 1000)
    {
        LogTraceC << " absorb proc time " << processingTimeUsec_;// << std::endl;
        
        int processingUsec = (processingTimeUsec_/1000)*1000;
        
        LogTraceC << " proc absorb part " << processingUsec;// << std::endl;
        
        if (processingUsec > playoutTimeUsec)
        {
            LogTraceC << " skip frame. proc " << processingUsec
            << " playout " << playoutTimeUsec;// << std::endl;
            
            processingUsec = playoutTimeUsec;
            playoutTimeUsec = 0;
        }
        else
            playoutTimeUsec -= processingUsec;
        
        processingTimeUsec_ = processingTimeUsec_ - processingUsec;
        LogTraceC << " playout usec " << playoutTimeUsec
                  << "us, total proc " << processingTimeUsec_ << "us" << std::endl;
    }
    
    framePlayoutTimeMs_ = playoutTimeUsec/1000;
}

void JitterTiming::run(boost::function<void()> callback)
{
    assert(framePlayoutTimeMs_ >= 0);
    //int64_t now = MtNdnUtils::microsecondTimestamp();
    LogTraceC << " timer wait " << framePlayoutTimeMs_ << "ms ... "
              << MtNdnUtils::microsecondTimestamp() << std::endl;
    //std::cout << "jt " << now << std::endl;
    //LOG(WARNING) << "[JT] " << MtNdnUtils::microsecondTimestamp();
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
    prevPlayoutTsUsec_ = 0;
}
