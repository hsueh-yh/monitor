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
                    const GeneralConsumerParams &consumerParams):
                    generalParams_(generalParams),
                    consumerParams_(consumerParams),
                    isConsuming_(false),
                    observer_(NULL)
{}

Consumer::~Consumer ()
{
    if( isConsuming_ )
        stop();

    LOG(INFO) << "[Consumer] dtor" << endl;
    LOG(WARNING) << "[Consumer] dtor" << endl;
}


int
Consumer::init(const ConsumerSettings &settings,
               const std::string &threadName)
{
    settings_ = settings;
    streamPrefix_ = settings_.streamPrefix_;
    //LOG(INFO) << "streamPrefix_: " << streamPrefix_ << std::endl;
    frameBuffer_.reset(new FrameBuffer());
    frameBuffer_->setLogger(logger_);
    frameBuffer_->setDescription(MtNdnUtils::formatString("%s-Buffer",
                                                       getDescription().c_str()));
    frameBuffer_->init();

    pipeliner_.reset(new Pipeliner(this));
    pipeliner_->setLogger(logger_);
    pipeliner_->setDescription(MtNdnUtils::formatString("%s-Pipeliner",
                                                     getDescription().c_str()));
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

    LOG(INFO) << "Consumer stop \n" << std::endl;

    return RESULT_OK;
}

int
Consumer::stop( int tmp )
{
    status_ = STOPED;

    pipeliner_->stop();
    frameBuffer_->stop();

    LOG(ERROR) << "[Consumer] Stop "
               << " Pipeliner:" << pipeliner_.use_count()
               << " FrameBuffer:" << frameBuffer_.use_count()
               << endl;
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
    if (observer_)
    {
        ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(observerMutex_);
        ConsumerStatus status;

        switch (newState) {
            case Pipeliner::StateWaitInitial:
                status = ConsumerStatusNoData;
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

            case Pipeliner::StateInactive:
            default:
                status = ConsumerStatusStopped;
                break;
        }

        observer_->onStatusChanged(status);
    }
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

        /*
        playoutThread_.push_back(startThread([this,adjustment]()->int{
            playout_->start(adjustment);
            try
            {
                VLOG(LOG_DEBUG) << "playoutThread "
                             << boost::this_thread::get_id() << std::endl;

                MtNdnUtils::getIoService().run();
                VLOG(LOG_DEBUG) << "playoutThread ioservice.run OVER "
                             << boost::this_thread::get_id() << std::endl;
            }
            catch (std::exception &e) // fatal
            {
                throw std::runtime_error(e.what());
            }
        }));
        */
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
