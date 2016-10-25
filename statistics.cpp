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
    if( delay_ == 0)
    {
        delay_ = delay;
    }
    else
    {
        delay_ = (double)delay_*(1-alpha_) + (double)delay*alpha_;
    }
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
{ return delay_; }

Statistics::Statistics():
    requestCounter_(0),
    receiveCounter_(0),
    retransmissionCounter_(0),
    lostCounter_(0),
    lostRate_(0.0),
    alpha_(1.0/4.0),
    delay_(0)
{}
