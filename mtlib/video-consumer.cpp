
#include "video-consumer.h"
#include "video-playout.h"


VideoConsumer::VideoConsumer(const GeneralParams &generalParams,
                             const ConsumerParams &consumerParams,
                             IExternalRenderer *const externalRenderer):
    Consumer(generalParams,consumerParams),
    decoder_(new VideoDecoder())
{
    setDescription("[VideoConsumer]\t");
    renderer_.reset(new ExternalVideoRendererAdaptor(externalRenderer));
    decoder_->setFrameConsumer(getRenderer().get());
}

VideoConsumer::~VideoConsumer()
{}


int
VideoConsumer::init(const ConsumerSettings &settings)
{
    if (RESULT_GOOD(Consumer::init(settings)))
    {
        pipeliner_->init();

        decoder_->init(logger_);
        //decoder_->init(((VideoThreadParams*)getCurrentThreadParameters())->coderParams_);

        playout_.reset(new VideoPlayout(this));
        playout_->setLogger(logger_);
        playout_->init(decoder_.get());
        //((VideoPlayout*)playout_.get())->onFrameSkipped_ = boost::bind(&VideoConsumer::onFrameSkipped, this, _1, _2, _3, _4, _5);

        VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                  << "initialized" << std::endl;

        return RESULT_OK;
    }

    return RESULT_ERR;
}

int
VideoConsumer::start()
{
   if( RESULT_GOOD(Consumer::start()) )
       VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                 << "started" << std::endl;
   else
       return RESULT_ERR;

   return RESULT_OK;
}

int
VideoConsumer::stop()
{
    if (RESULT_GOOD(Consumer::stop()))
        VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                  << "stopped" << std::endl;
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
