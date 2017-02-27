#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include "ff-decoder.h"
#include "renderer.h"
#include "mtndn-object.h"
#include "params.h"


class IEncodedFrameConsumer
{
public:
    virtual void
    onEncodedFrameDelivered(AVPacket &encodedImage,
                            double captureTimestamp,
                            bool completeFrame = true) = 0;
};

class VideoDecoder: public IEncodedFrameConsumer,
                    public FF_DecodedImageCallback,
                    public MtNdnComponent
{
public:
    VideoDecoder();
    ~VideoDecoder() { }

    void setFrameConsumer(IRawFrameConsumer *frameConsumer)
    { decodedFrameConsumer_ = frameConsumer; }

    int init();
    void reset();

    /**
     * @brief interface conformance - IEncodedFrameConsumer,
     *          decode frame and callback VideoDecoder::onDecoded()
     * @param
     * @return
     */
    void onEncodedFrameDelivered(AVPacket &encodedImage,
                                 double timestamp,
                                 bool completeFrame);
private:
    VideoCoderParams settings_;
    IRawFrameConsumer *decodedFrameConsumer_;
    std::shared_ptr<Decoder> decoder_;
    double capturedTimestamp_ = 0;
    int frameCount_;

    int
    resetDecoder();

    // interface conformance - DecodedImageCallback
    void onDecoded(const AVFrame &decodedImage);
};

#endif // VIDEODECODER_H
