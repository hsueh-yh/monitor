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

    bool requestFrame(PacketNumber frameNo);

    void
    registerCallback(IPipelinerCallback* callback)
    { callback_ = callback; }


protected:

    //FILE *pipelinerFIle_;
    Statistics *statistic;
    bool isRetransmission;

    int count_;
    unsigned int reqCurPktNo_, reqLastNo_;


    //******************************************************************************
    void
    switchToState(Pipeliner::State newState)
    {
        State oldState = state_;
        if( oldState != newState )
        {
            state_ = newState;
            LOG(INFO) << "[Pipeliner] change state " <<  state2string(state_)
                      << " to " << state2string(newState) << endl;
        }
        if (callback_)
            callback_->onStateChanged(oldState, state_);
    }

    Pipeliner::State
    getState()
    { return state_;  }

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

    std::string state2string( State state )
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

#endif /*PIPELINER_H_ */
