//
//  ndnrtc-object.cpp
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//  Created: 8/21/13
//

#include <stdarg.h>
#include <boost/thread/lock_guard.hpp>
#include <boost/chrono.hpp>


#include "object.h"
#include "utils.h"

using namespace std;

using namespace boost;

//******************************************************************************
/**
  *@name NdnRtcComponent class
 */
NdnRtcComponent::NdnRtcComponent():
    watchdogTimer_(NdnUtils::getIoService()),
    isJobScheduled_(false),
    isTimerCancelled_(false)
{}

NdnRtcComponent::NdnRtcComponent(INdnRtcComponentCallback *callback):
    callback_(callback),
    watchdogTimer_(NdnUtils::getIoService()),
    isJobScheduled_(false),
    isTimerCancelled_(false)
{}

NdnRtcComponent::~NdnRtcComponent()
{
    try {
        stopJob();
    }
    catch (...) {
    }
}


void NdnRtcComponent::onError(const char *errorMessage, const int errorCode)
{
    if (hasCallback())
    {
        lock_guard<mutex> scopedLock(callbackMutex_);
        callback_->onError(errorMessage, errorCode);
    }
    else
    {
		cout << "error" << endl;
        //LogErrorC << "error occurred: " << string(errorMessage) << endl;
        //if (logger_) logger_->flush();
    }
}

thread
NdnRtcComponent::startThread(boost::function<bool ()> threadFunc)
{
    thread threadObject = thread([threadFunc](){
        bool result = false;
        do {
            try
            {
                this_thread::interruption_point();
                {
                    this_thread::disable_interruption di;
                    result = threadFunc();
                }
            }
            catch (thread_interrupted &interruption)
            {
                result = false;
            }
        } while (result);
    });
    
    return threadObject;
}

void
NdnRtcComponent::stopThread(thread &threadObject)
{
    threadObject.interrupt();
    
    if (threadObject.joinable())
    {
        bool res = threadObject.try_join_for(boost::chrono::milliseconds(500));
                                             
        if (!res)
            threadObject.detach();
    }
}


void NdnRtcComponent::scheduleJob(const unsigned int usecInterval,
                                  boost::function<bool()> jobCallback)
{
    boost::lock_guard<boost::recursive_mutex> scopedLock(this->jobMutex_);
    
    watchdogTimer_.expires_from_now(std::chrono::microseconds(usecInterval));
    //watchdogTimer_.expires_from_now(boost::posix_time::milliseconds(usecInterval));
    //watchdogTimer_.expires_from_now(boost::chrono::microseconds(usecInterval));
    isJobScheduled_ = true;
    isTimerCancelled_ = false;
    
    watchdogTimer_.async_wait([this, usecInterval, jobCallback](const boost::system::error_code &code){
        if (code != boost::asio::error::operation_aborted)
        {
            if (!isTimerCancelled_)
            {
                isJobScheduled_ = false;
                boost::lock_guard<boost::recursive_mutex> scopedLock(this->jobMutex_);
                bool res = jobCallback();
                if (res)
                    this->scheduleJob(usecInterval, jobCallback);
            }
        }
    });

}

void NdnRtcComponent::stopJob()
{
    NdnUtils::performOnBackgroundThread([this]()->void{
        boost::lock_guard<boost::recursive_mutex> scopedLock(jobMutex_);
        watchdogTimer_.cancel();
        isTimerCancelled_ = true;
    });
    
    isJobScheduled_ = false;
}

