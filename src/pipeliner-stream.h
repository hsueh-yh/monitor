/*
  *PipelinerStream.h
 *
  * Created on: Jun 15, 2016
  *     Author: xyh
 *
  * PipelinerStream 收发数据包
 *
  * PipelinerStreamWindow 发送窗口，控制网络中Interest（未收到Data）的数量
 */

#ifndef PipelinerStream_STREAM_H_
#define PipelinerStream_STREAM_H_

#include "pipeliner.h"

using namespace ndn;

class PipelinerStream : public Pipeliner
{
public:

    PipelinerStream(const Consumer *consumer);

    ~PipelinerStream();

    //******************************************************************************

    int
    init();

    int
    start();

    int
    stop();


protected:

    unsigned int interestLifeTimeMS;


    //******************************************************************************
    ptr_lib::shared_ptr<Interest>
    getDefaultInterest();


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
    request();

};


#endif /*PipelinerStream_H_ */
