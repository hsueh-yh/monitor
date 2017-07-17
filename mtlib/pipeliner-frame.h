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

using namespace ndn;

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
