#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <iostream>
#include <string>
#include "logger.hpp"

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

    int64_t delay_;
    double alpha_;

};


#endif // STATISTICS_HPP
