#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <iostream>
#include <string>
#include <mutex>

#include "glogger.h"

class Statistics
{
public:

    static Statistics *getInstance();

    void addRequest();

    void addData(int64_t delay);

    void markMiss();

    void retransmission();

    double getLostRate();

    int64_t getDelay();

    int64_t getLastRecvDataTs()
    {
        return lastRecvDataTS_;
    }
    int64_t getDataInterval( int64_t newDataTs )
    {
        int64_t diff = (lastRecvDataTS_ == 0 ? 0 : newDataTs - lastRecvDataTS_);
        lastRecvDataTS_ = newDataTs;
        return diff;
    }

    void setLastRecvDataTs( int64_t ts )
    {
        lastRecvDataTS_ = ts;
    }

private:
    Statistics();

    static Statistics *instance_;

    int requestCounter_;
    int receiveCounter_;

    int retransmissionCounter_;

    int lostCounter_;
    double lostRate_;

    int64_t avgDelay_;
    int64_t lastRecvDataTS_;
    double alpha_;

    int counterStepSize_;
    int delayCounter[100];

    std::mutex mutex_;

};


#endif // STATISTICS_HPP
