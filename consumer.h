#ifndef CONSUMER_H_
#define CONSUMER_H_

#include <pthread.h>
#include <ndn-cpp/face.hpp>
#include <boost/shared_ptr.hpp>

#include <cstdlib>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <thread>

#include "util/CircularQueue.h"
#include "utils.h"
#include "object.h"
#include "face-wrapper.h"
#include "frame-buffer.h"
#include "pipeliner.h"
#include "player.h"

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

    Consumer (const char *uri,boost::shared_ptr<FaceWrapper> faceWrapper);

	~Consumer ();

	void init();

	void start();

    void stop();

	boost::shared_ptr<FaceWrapper> faceWrapper_;
	boost::shared_ptr<FrameBuffer> frameBuffer_;
	boost::shared_ptr<Pipeliner> pipeliner_;
    boost::shared_ptr<Player> player_;

private:

    Name *name;
    int callbackCount_;

    Status status_;

};

#endif //CONSUMER_H_
