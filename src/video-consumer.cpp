
#include "video-consumer.h"
#include "video-playout.h"


VideoConsumer::VideoConsumer(const GeneralParams &generalParams,
                             const GeneralConsumerParams &consumerParams,
                             IExternalRenderer *const externalRenderer):
    Consumer(generalParams,consumerParams),
    decoder_(new VideoDecoder())
{
    renderer_.reset(new ExternalVideoRendererAdaptor(externalRenderer));
    decoder_->setFrameConsumer(getRenderer().get());
}

VideoConsumer::~VideoConsumer()
{}


int
VideoConsumer::init(const ConsumerSettings &settings,
     const std::string &threadName)
{
    if (RESULT_GOOD(Consumer::init(settings, threadName)))
    {
        pipeliner_->init();

        decoder_->init();
        //decoder_->init(((VideoThreadParams*)getCurrentThreadParameters())->coderParams_);

        playout_.reset(new VideoPlayout(this));
        playout_->setLogger(logger_);
        playout_->init(decoder_.get());
        //((VideoPlayout*)playout_.get())->onFrameSkipped_ = boost::bind(&VideoConsumer::onFrameSkipped, this, _1, _2, _3, _4, _5);

        LOG(INFO) << "VideoConsumer initialized" << std::endl;

        return RESULT_OK;
    }

    return RESULT_ERR;
}

int
VideoConsumer::start()
{
   if( RESULT_GOOD(Consumer::start()) )
       LOG(INFO) << "Started" << std::endl;
   else
       return RESULT_ERR;

   return RESULT_OK;
}

int
VideoConsumer::stop()
{
    if (RESULT_GOOD(Consumer::stop()))
        LOG(INFO) << "Stopped" << std::endl;
    else
        return RESULT_ERR;

    return RESULT_OK;
}

void
VideoConsumer::onStateChanged(const int &oldState, const int &newState)
{
    Consumer::onStateChanged(oldState, newState);
}

void
VideoConsumer::playbackEventOccurred(PlaybackEvent event,
                                     unsigned int frameSeqNo)
{
    if (observer_)
    {
        ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(observerMutex_);
        observer_->onPlaybackEventOccurred(event, frameSeqNo);
    }
}

void
VideoConsumer::triggerRebuffering()
{
    Consumer::triggerRebuffering();
    decoder_->reset();
}


void
VideoConsumer::onFrameSkipped(PacketNumber playbackNo, PacketNumber sequenceNo,
                              PacketNumber pairedNo, bool isKey,
                              double assembledLevel)
{
}

/*
void
VideoConsumer::onTimeout(const shared_ptr<const Interest> &interest)
{
}
*/
