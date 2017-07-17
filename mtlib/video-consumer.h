#ifndef VIDEOCONSUMER_H
#define VIDEOCONSUMER_H

#include "consumer.h"
#include "video-renderer.h"
#include "video-decoder.h"

class VideoConsumer : public Consumer
{
public:
    VideoConsumer(const GeneralParams &generalParams,
                  const ConsumerParams &consumerParams,
                  IExternalRenderer *const externalRenderer);
    virtual ~VideoConsumer();

    int
    init(const ConsumerSettings &settings);

    int
    start();

    int
    stop();

//    void
//    setLogger(ndnlog::new_api::Logger *logger);

    void
    onStateChanged(const int &oldState, const int &newState);

    /**
      *Called by video playout mechanism to notify consumer observer
      *about new playback events
     */
    void
    playbackEventOccurred(PlaybackEvent event, unsigned int frameSeqNo);

    void
    triggerRebuffering();

private:
    std::shared_ptr<VideoDecoder> decoder_;

    std::shared_ptr<IVideoRenderer>
    getRenderer()
    { return std::dynamic_pointer_cast<IVideoRenderer>(renderer_); }

    void
    onFrameSkipped(PacketNumber playbackNo, PacketNumber sequenceNo,
                   PacketNumber pairedNo, bool isKey,
                   double assembledLevel);

    /*
    void
    onTimeout(const boost::shared_ptr<const Interest> &interest);
    */
};


#endif // VIDEOCONSUMER_H
