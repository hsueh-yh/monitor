#include "statistics.hpp"

Statistics * Statistics::instance_=NULL;

Statistics *Statistics::getInstance()
{
    if( instance_ == NULL )
    {
        instance_ = new Statistics();
    }
    return instance_;
}

void Statistics::addRequest()
{ ++requestCounter_; }

void Statistics::addData(int64_t delay)
{
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
    if( idx >= 0 && idx <= 99 )
    {
        ++delayCounter[idx];
        for( int j = 0; j < 100; ++j )
            LOG_IF(INFO,delayCounter[j] > 0)
                    << "Delay[" << j*counterStepSize_<<","<<((j+1)*counterStepSize_-1)<<"]"
                    << " : " << delayCounter[j];
    }
    else
        LOG(WARNING) << "[Statistics] Delay=" << delay << std::endl;
}

void Statistics::markMiss()
{
    ++lostCounter_;
    lostRate_ = (double)lostCounter_ / (double)requestCounter_;
}

void Statistics::retransmission()
{ ++retransmissionCounter_; }

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
    avgDelay_(0)
{
    for( int i = 0; i < 100; ++i )
        delayCounter[i] = 0;
}
