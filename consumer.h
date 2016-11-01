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
#include "statistics.hpp"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;


#define WIDTH 640
#define HEIGHT 480


class Consumer
{
public:

    typedef enum{
        STOPED = -1,
        READY = 0,
        STARTED = 1,

    }Status;

    Consumer (string uri, ptr_lib::shared_ptr<FaceWrapper> faceWrapper);

    ~Consumer ();

    void init();

    void start();

    void stop();

    Status getstatus()
    { return status_;}

    ptr_lib::shared_ptr<FaceWrapper> faceWrapper_;
    ptr_lib::shared_ptr<FrameBuffer> frameBuffer_;
    ptr_lib::shared_ptr<Pipeliner> pipeliner_;
    ptr_lib::shared_ptr<Player> player_;
    static Statistics statistic;

private:

    //Name *name;
    std::string prefix_;
    int callbackCount_;


    Status status_;

};

#endif //CONSUMER_H_
