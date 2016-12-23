#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <iostream>
#include <string>
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

private:
    Statistics();

    static Statistics *instance_;

    int requestCounter_;
    int receiveCounter_;

    int retransmissionCounter_;

    int lostCounter_;
    double lostRate_;

    int64_t avgDelay_;
    double alpha_;

    int counterStepSize_;
    int delayCounter[100];

};


#endif // STATISTICS_HPP
