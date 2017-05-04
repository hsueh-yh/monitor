#include "statistics.h"

Statistics  *Statistics::instance_=new Statistics();

Statistics *Statistics::getInstance()
{
    /*
    if( instance_ == NULL )
    {
        std::lock_guard<std::mutex> lockguard(mutex_);
        if( instance_ == NULL )
            instance_ = new Statistics();
    }
    */
    return instance_;
}

void Statistics::addRequest()
{
    std::lock_guard<std::mutex> lockguard(mutex_);
    ++requestCounter_;
}

void Statistics::addData(int64_t delay)
{
    std::lock_guard<std::mutex> lockguard(mutex_);
    ++receiveCounter_;
    if( avgDelay_ == 0)
    {
        avgDelay_ = delay;
    }
    else
    {
        avgDelay_ = (double)avgDelay_*(1-alpha_) + (double)delay*alpha_;
    }

    int idx = (delay/1000) / counterStepSize_;
    if( idx >= 0 &&idx <= 99 )
    {
        ++delayCounter[idx];
        /*
        for( int j = 0; j < 100; ++j )
            LOG_IF(INFO,delayCounter[j] > 0)
                    << "Delay[" << j*counterStepSize_<<","<<((j+1)*counterStepSize_-1)<<"]"
                    << " : " << delayCounter[j];
        */
    }
    else
        LOG(WARNING) << "[Statistics] Delay = " << delay/1000 << "ms" << std::endl;
}

void Statistics::markMiss()
{
    std::lock_guard<std::mutex> lockguard(mutex_);
    ++lostCounter_;
    if( requestCounter_ == 0 )
        ++requestCounter_;
    lostRate_ = (double)lostCounter_ / (double)requestCounter_;
}

void Statistics::retransmission()
{
    std::lock_guard<std::mutex> lockguard(mutex_);
    ++retransmissionCounter_;
}

double Statistics::getLostRate()
{ return lostRate_; }

int64_t Statistics::getDelay()
{ return avgDelay_; }

Statistics::Statistics():
    requestCounter_(0),
    receiveCounter_(0),
    retransmissionCounter_(0),
    lostCounter_(0),
    lostRate_(0.0),
    alpha_(1.0/2.0),
    counterStepSize_(5),
    avgDelay_(0),
    lastRecvDataTS_(0)
{
    for( int i = 0; i < 100; ++i )
        delayCounter[i] = 0;
}
