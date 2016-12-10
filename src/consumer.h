#ifndef CONSUMER_H_
#define CONSUMER_H_

#include <pthread.h>
#include <ndn-cpp/face.hpp>

#include <cstdlib>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <thread>

#include "utils.h"
#include "object.h"
#include "face-wrapper.h"
#include "frame-buffer.h"
#include "pipeliner.h"
#include "player.h"
#include "playout.h"
#include "statistics.h"
#include "params.h"
#include "renderer.h"
#include "interfaces.h"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;


#define WIDTH 640
#define HEIGHT 480

class ConsumerSettings {
public:
    std::string streamPrefix_;
    MediaStreamParams streamParams_;
    ptr_lib::shared_ptr<FaceProcessor> faceProcessor_;
};


class Consumer: public IPipelinerCallback,
                public NdnRtcComponent
{
public:

    typedef enum{
        STOPED = -1,
        READY = 0,
        STARTED = 1
    }Status;

    Consumer (int id, string uri, ptr_lib::shared_ptr<FaceWrapper> faceWrapper);

    Consumer (const GeneralParams &generalParams,
              const GeneralConsumerParams &consumerParams);
    virtual ~Consumer ();


    /**
      *@brief init FrameBuffer Pipeliner and Renderer
      *@param settings, ConsumerSettings
      *@param threadName, consumer thread name
      *@return
     */
    virtual int
    init(const ConsumerSettings &settings, const string &threadName);

    virtual int
    start();

    virtual int
    stop();
    int
    stop( int tmp );

    const std::string
    getPrefix() const
    {
        LOG(INFO) << "getPrefix " << streamPrefix_ << std::endl;
        return streamPrefix_;
    }

    Status
    getstatus()
    { return status_;}

    bool
    getIsConsuming()
    { return isConsuming_; }

    ptr_lib::shared_ptr<FrameBuffer>
    getFrameBuffer()
    { return frameBuffer_; }

    ptr_lib::shared_ptr<FaceWrapper>
    getFace()
    { return settings_.faceProcessor_->getFaceWrapper(); }

    virtual GeneralParams
    getGeneralParameters() const
    { return generalParams_; }

    //  IPipelinerCallback
    //***********************************************************
    virtual void
    onBufferingEnded();

    virtual void
    onInitialDataArrived();

    virtual void
    onStateChanged(const int &oldState, const int &newState);
    //***********************************************************

    void
    triggerRebuffering();

    void
    registerObserver(IConsumerObserver *const observer)
    {
        ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(observerMutex_);
        observer_ = observer;
    }

    void
    unregisterObserver()
    {
        ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(observerMutex_);
        observer_ = NULL;
    }

    ptr_lib::shared_ptr<FaceWrapper> faceWrapper_;
    ptr_lib::shared_ptr<FrameBuffer> frameBuffer_;
    ptr_lib::shared_ptr<Pipeliner> pipeliner_;
    ptr_lib::shared_ptr<Player> player_;
    ptr_lib::shared_ptr<Playout> playout_;
    static Statistics statistic;

protected:

    bool isConsuming_;
    GeneralParams generalParams_;
    GeneralConsumerParams consumerParams_;
    ConsumerSettings settings_;

    //Name *name;
    std::string streamPrefix_;
    int callbackCount_;

    Status status_;

    ptr_lib::shared_ptr<IRenderer> renderer_;

    ptr_lib::mutex observerMutex_;
    IConsumerObserver *observer_;

};

#endif //CONSUMER_H_
