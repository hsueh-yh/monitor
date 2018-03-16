/*
  *Pipeliner.h
 *
  * Created on: Jun 15, 2016
  *     Author: xyh
 *
  * Pipeliner 收发数据包
 *
  * PipelinerWindow 发送窗口，控制网络中Interest（未收到Data）的数量
 */

#ifndef PIPELINER_FRAME_H_
#define PIPELINER_FRAME_H_

#include "pipeliner.h"

#include <map>
#include <string>
#include <uuid/uuid.h>

using namespace ndn;

class CsTimerQueue
{
public:

    CsTimerQueue(){}
    ~CsTimerQueue(){}

    void recvData(Data& data, const unsigned int delay)
    {
        const uint8_t *buf = data.getContent().buf();
        string cs_id((const char*)buf, 36);
        LOG(INFO) << "From CS: {" << cs_id << "}" << endl;
        updateTimer(delay, timer_queue[cs_id]);
    }

    void sendInterest()
    {
        //strategy
    }

    void updateTimer(const unsigned int delay, unsigned int& timer, double alpha = 0.1)
    {
        timer = (1.0-alpha)*timer + alpha*delay;
    }

private:
    //
    std::map<std::string, unsigned int> timer_queue;
};

class PipelinerFrame : public Pipeliner
{
public:

    PipelinerFrame(const Consumer *consumer);

    ~PipelinerFrame();

    //******************************************************************************

    int
    init();

    int
    start();

    int
    stop();

    void
    registerCallback(IPipelinerCallback* callback)
    { callback_ = callback; }


protected:

    //******************************************************************************
    ptr_lib::shared_ptr<Interest>
    getDefaultInterest(PacketNumber frameNo);

    CsTimerQueue    cs_queue;


    //******************************************************************************
    void
    onData(const ptr_lib::shared_ptr<const Interest> &interest, const ptr_lib::shared_ptr<Data> &data);

    void
    onTimeout(const ptr_lib::shared_ptr<const Interest> &interest);

    void
    requestMeta();

    void
    requestNextPkt();

    bool
    requestFrame(PacketNumber frameNo);

};

#endif /*PIPELINER_H_ */
