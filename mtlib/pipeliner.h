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

    bool
    dataArrived(PacketNumber packetNo);

    double
    dataMissed(PacketNumber packetNo);

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
    std::set<PacketNumber> missedPool_;
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

    virtual int
    init();

    virtual int
    start();

    virtual int
    stop();

    virtual int
    restart();

    virtual void
    express(const Name &name);

    virtual void
    express(const Interest &interest);

    virtual void
    registerCallback(IPipelinerCallback* callback)
    { callback_ = callback; }

    virtual void
    setDescription(const std::string& desc)
    { description_ = desc; }

    virtual std::string
    getDescription() const
    {
        if (description_ == "")
        {
            std::stringstream ss;
            ss << "MtNdnObject"
               /*<< std::hex << this*/;
            return ss.str();
        }

        return description_;
    }


protected:

    State state_;
    mutable boost::shared_mutex state_mutex_;

    Name streamName_;

    Consumer *consumer_;
    ptr_lib::shared_ptr<FaceWrapper> face_;
    ptr_lib::shared_ptr<FrameBuffer> frameBuffer_;
    PipelinerWindow window_;
    IPipelinerCallback* callback_;

    //FILE *pipelinerFIle_;
    Statistics *statistic;
    bool isRetransmission;

    int request_count_;
    unsigned int reqCurPktNo_, lastFrmNo_;


    //******************************************************************************
    void
    switchToState(Pipeliner::State newState)
    {
        if( newState == getState() )
            return ;

        State oldState = getState();
        {
            boost::lock_guard<boost::shared_mutex> lock(state_mutex_);
            if( newState == state_ )
                return ;
            oldState = state_;
            if( oldState != newState )
            {
                state_ = newState;
                VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                          << "change state '" <<  state2string(state_)
                          << "' -> '" << state2string(newState) <<"'"<< endl;
            }
        }
        if (callback_)
            callback_->onStateChanged(oldState, state_);
    }

    Pipeliner::State
    getState()
    {
        boost::shared_lock<boost::shared_mutex> lock(state_mutex_);
        return state_;
    }

    ptr_lib::shared_ptr<Interest>
    getDefaultInterest(const Name &prefix);

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

    virtual void
    reset();

    //******************************************************************************
    virtual void
    onData(const ptr_lib::shared_ptr<const Interest> &interest, const ptr_lib::shared_ptr<Data> &data);

    virtual void
    onTimeout(const ptr_lib::shared_ptr<const Interest> &interest);

    /*
    void
    requestMeta();

    void
    requestNextPkt();

    bool
    request(unsigned int delay = 5*1000);
*/
};


#endif /*PIPELINER_H_ */
