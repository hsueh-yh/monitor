/*
 * Pipeliner.h
 *
 *  Created on: Jun 15, 2016
 *      Author: xyh
 *
 *  Pipeliner 收发数据包
 *
 *  PipelinerWindow 发送窗口，控制网络中Interest（未收到Data）的数量
 */

#ifndef PIPELINER_H_
#define PIPELINER_H_

#include <ndn-cpp/name.hpp>
#include <ndn-cpp/data.hpp>

#include "common.h"
#include "frame-buffer.h"
#include "face-wrapper.h"
#include "statistics.hpp"

using namespace ndn;


class PipelinerWindow
{
public:
    PipelinerWindow();
    ~PipelinerWindow();

    void
    init(unsigned int windowSize/*, const FrameBuffer* frameBuffer*/);

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
    const FrameBuffer* frameBuffer_;
};


class Pipeliner
{
public:

    typedef enum{
        Stoped = -1,
        Ready = 0,
        Bootstrap = 1,
        Fetching = 2
    }State;

    Pipeliner( std::string prefix);

    ~Pipeliner();

    //******************************************************************************

    void init(ptr_lib::shared_ptr<FrameBuffer> frameBuffer, ptr_lib::shared_ptr<FaceWrapper> faceWrapper);

    void express(Name& name);

    void express(Interest& interest);

    void requestFrame(PacketNumber frameNo);

    void startFetching();

    void stop();

    void changetoState(Pipeliner::State stat);

    Pipeliner::State getState()
    { lock(); Pipeliner::State stat = state_; unlock(); return stat;  }

    ptr_lib::shared_ptr<Interest>
    getDefaultInterest(const Name& prefix, int64_t timeoutMs = 0);

    //******************************************************************************
    void onData(const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& data);

    void onTimeout(const ptr_lib::shared_ptr<const Interest>& interest);


    void
    lock()  { syncMutex_.lock(); }

    void
    unlock() { syncMutex_.unlock(); }

private:

    Name basePrefix_;

    ptr_lib::shared_ptr<FaceWrapper> faceWrapper_;
    ptr_lib::shared_ptr<FrameBuffer> frameBuffer_;
    PipelinerWindow window_;

    //FILE *pipelinerFIle_;
    Statistics *statistic;
    bool isRetransmission;

    int count_;
    unsigned int requestPktNo_;
    State state_;
    std::recursive_mutex syncMutex_;
};


#endif /* PIPELINER_H_ */
