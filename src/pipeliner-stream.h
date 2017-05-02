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

    bool
    requestFrame(PacketNumber frameNo, bool isExpressInterest=true);


protected:

    unsigned int interestLifeTimeMS;
    int request_count_;
    unsigned int reqCurPktNo_, reqLastNo_;


    //******************************************************************************
    void
    switchToState(PipelinerStream::State newState)
    {
        State oldState = state_;
        if( oldState != newState )
        {
            state_ = newState;
            LOG(INFO) << "[PipelinerStream] change state " <<  state2string(state_)
                      << " to " << state2string(newState) << endl;
        }
        if (callback_)
            callback_->onStateChanged(oldState, state_);
    }

    PipelinerStream::State
    getState()
    { return state_;  }

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

    std::string
    state2string( State state )
    {
        std::string str = "";
        switch(state)
        {
        case StateInactive:
            str = "Inactive";
            break;
        case StateWaitInitial:
            str = "StateWaitInitial";
            break;
        case StateBootstrap:
            str = "StateBootstrap";
            break;
        case StateFetching:
            str = "StateFetching";
            break;
        default:
            break;
        }
        return str;
    }

};


#endif /*PipelinerStream_H_ */
