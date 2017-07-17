#include "consumer.h"
#include <unistd.h>


Consumer::Consumer (int id, std::string uri, ptr_lib::shared_ptr<FaceWrapper> faceWrapper) :
    callbackCount_ ( 0 ),
    status_(STOPED),
    streamPrefix_(uri),
    //name(new Name(uri)),
    faceWrapper_(faceWrapper)
{}

Consumer::Consumer (const GeneralParams &generalParams,
                    const ConsumerParams &consumerParams):
                    generalParams_(generalParams),
                    consumerParams_(consumerParams),
                    isConsuming_(false),
                    observer_(NULL)
{
    setDescription("[Consumer]\t");
    VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
              << "ctor" << endl;
}

Consumer::~Consumer ()
{
    if( isConsuming_ )
        stop();

    VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
              << "dtor" << endl;
}


int
Consumer::init(const ConsumerSettings &settings)
{
    VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
              << "init..." << endl;

    settings_ = settings;
    streamPrefix_ = settings_.streamPrefix_;
    //LOG(INFO) << "streamPrefix_: " << streamPrefix_ << std::endl;
    frameBuffer_.reset(new FrameBuffer());
    frameBuffer_->setLogger(logger_);
    //frameBuffer_->setDescription(MtNdnUtils::formatString("[FrameBuffer]"));
    frameBuffer_->init();

    if( settings.transType_ == "byFrame" )
        pipeliner_.reset(new PipelinerFrame(this));
    else
        pipeliner_.reset(new PipelinerStream(this));

    pipeliner_->setLogger(logger_);
    //pipeliner_->setDescription(MtNdnUtils::formatString("%s-Pipeliner",
    //                                                 getDescription().c_str()));
    pipeliner_->registerCallback(this);

    renderer_->init();

    if( observer_ )
    {
        ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(observerMutex_);
        observer_->onStatusChanged(ConsumerStatusStopped);
    }

    return RESULT_OK;
}

int
Consumer::start()
{
    isConsuming_ = true;

    if( RESULT_FAIL(pipeliner_->start()) )
    {
        isConsuming_ = false;
        return RESULT_ERR;
    }

    if( observer_ )
    {
        ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(observerMutex_);
        observer_->onStatusChanged(ConsumerStatusNoData);
    }
    return RESULT_OK;
}

int
Consumer::stop()
{
    MtNdnUtils::performOnBackgroundThread([this]()->void{
            isConsuming_ = false;
            pipeliner_->stop();
            playout_->stop();
            renderer_->stopRendering();
            std::vector<boost::thread>::iterator iter = playoutThread_.begin();
            while( iter != playoutThread_.end() )
            {
                stopThread(*iter);
            }
    });

    VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                   << "stop" << std::endl;

    return RESULT_OK;
}

int
Consumer::stop( int tmp )
{
    status_ = STOPED;

    pipeliner_->stop();
    frameBuffer_->stop();

    VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                   << "stop " << tmp << std::endl;
    //faceWrapper_->shutdown();

    /*
    player_->~Player();
    player_.reset();

    pipeliner_->stop();
    pipeliner_->~Pipeliner();
    pipeliner_.reset();

    frameBuffer_->~FrameBuffer();
    */
    //frameBuffer_.reset();

//    if( faceWrapper_.use_count() <= 1 )
//        faceWrapper_.reset();
}

void
Consumer::onStateChanged(const int &oldState, const int &newState)
{
    ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(observerMutex_);
    ConsumerStatus status;

    switch (newState) {

        case Pipeliner::StateInactive:
            status = ConsumerStatusStopped;
            if( oldState == Pipeliner::StateFetching )
            {
                playout_->stop();
                frameBuffer_->init();
            }
            break;

        case Pipeliner::StateWaitInitial:
            status = ConsumerStatusNoData;
            // pipeliner restart
            if( oldState == Pipeliner::StateFetching )
            {
                playout_->stop();
                frameBuffer_->init();
            }
            break;
        /*
        case Pipeliner::StateAdjust:
            status = ConsumerStatusAdjusting;
            break;

        case Pipeliner::StateBuffering:
            status = ConsumerStatusBuffering;
            break;
        */
        case Pipeliner::StateFetching:
            status = ConsumerStatusFetching;
            break;

        default:
            status = ConsumerStatusStopped;
            break;
    }

    if (observer_)
        observer_->onStatusChanged(status);
}

void
Consumer::setLogger(ndnlog::new_api::Logger *logger)
{
    if (frameBuffer_.get())
        frameBuffer_->setLogger(logger);

    if (pipeliner_.get())
        pipeliner_->setLogger(logger);

    if (playout_.get())
        playout_->setLogger(logger);

    ILoggingObject::setLogger(logger);
}

void
Consumer::setDescription(const std::string &desc)
{
    ILoggingObject::setDescription(desc);
}

void
Consumer::onBufferingEnded()
{
    // start rendering first, as playout may supply frames
    // right after "start playout" has been called
    if (!renderer_->isRendering())
        renderer_->startRendering(settings_.streamParams_.streamName_);

    if (!playout_->isRunning())
    {
        int adjustment = 0;
        playout_->start(adjustment);
    }
}

void
Consumer::onInitialDataArrived()
{
    if (observer_)
    {
        ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(observerMutex_);
        observer_->onStatusChanged(ConsumerStatusAdjusting);
    }
}

void
Consumer::triggerRebuffering()
{}
