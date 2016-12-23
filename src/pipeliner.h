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

#ifndef PIPELINER_H_
#define PIPELINER_H_

#include <ndn-cpp/name.hpp>
#include <ndn-cpp/data.hpp>

#include "mtndn-common.h"
#include "frame-buffer.h"
#include "face-wrapper.h"
#include "statistics.h"

using namespace ndn;


class Consumer;

class IPipelinerCallback
{
public:
    virtual void
    onBufferingEnded() = 0;

    virtual void
    onInitialDataArrived() = 0;

    virtual void
    onStateChanged(const int& oldState, const int& newState) = 0;
};

class PipelinerWindow
{
public:
    PipelinerWindow();
    ~PipelinerWindow();

    void
    init(unsigned int windowSize/*, const FrameBuffer *frameBuffer*/);

    void
    reset();

    void
    dataArrived(PacketNumber packetNo);

    bool
    canAskForData(PacketNumber packetNo);

    unsigned int
    getDefaultWindowSize();

    int
    getCurrentWindowSize();

    int
    changeWindow(int delta);

    bool
    isInitialized()
    { return isInitialized_; }

private:
    unsigned int dw_;   // default window size
    int w_;             // current window size
    bool isInitialized_;
    PacketNumber lastAddedToPool_;
    std::mutex mutex_;
    std::set<PacketNumber> framePool_;
    const FrameBuffer *frameBuffer_;
};

class Pipeliner : public MtNdnComponent
{
public:

    typedef enum{
        StateInactive = -1,
        StateWaitInitial = 0,
        StateBootstrap = 1,
        StateFetching = 2
    }State;

    Pipeliner(const Consumer *consumer);

    ~Pipeliner();

    //******************************************************************************

    int
    init();

    int
    start();

    int
    stop();

    void
    express(const Name &name);

    void
    express(const Interest &interest);

    bool requestFrame(PacketNumber frameNo);

    void
    registerCallback(IPipelinerCallback* callback)
    { callback_ = callback; }


protected:

    State state_;
    Name streamName_;

    Consumer *consumer_;
    ptr_lib::shared_ptr<FaceWrapper> face_;
    ptr_lib::shared_ptr<FrameBuffer> frameBuffer_;
    PipelinerWindow window_;
    IPipelinerCallback* callback_;

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
    getDefaultInterest(const Name &prefix);


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
