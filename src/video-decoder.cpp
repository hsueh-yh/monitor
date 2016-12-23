#include "video-decoder.h"
#include "include/mtndn-defines.h"
#include "glogger.h"
#include "mtndn-utils.h"


//********************************************************************************
#pragma mark - construction/destruction
VideoDecoder::VideoDecoder() :
    decodedFrameConsumer_(NULL),
    decoder_()
{
    decoder_.reset(new Decoder());
    std::stringstream ss;
    ss << componentId_;
    setDescription(ss.str());
    setDescription("VideoDecoder");
    LOG(INFO) << getDescription() << std::endl;
}

//********************************************************************************
#pragma mark - public
int VideoDecoder::init(/*const VideoCoderParams &settings*/)
{
    //settings_ = settings;
    if (RESULT_GOOD(resetDecoder()))
        return RESULT_OK;

    return RESULT_ERR;
}

void VideoDecoder::reset()
{
    if (frameCount_ > 0)
    {
        VLOG(LOG_TRACE) << "resetting decoder..." << std::endl;
        //int res = decoder_->Release();
        resetDecoder();
        VLOG(LOG_TRACE) << "decoder reset" << std::endl;
    }
}

//********************************************************************************
#pragma mark - intefaces realization DecodedImageCallback
void VideoDecoder::onDecoded(const AVFrame &decodedImage)
{
    /*
    VLOG(LOG_TRACE) << "decode Frame "
              << decodedImage.width
              << "*"
              << decodedImage.height
              << std::endl;
              */
    if (decodedFrameConsumer_)
        decodedFrameConsumer_->onDeliverFrame(decodedImage, capturedTimestamp_);

    //return 0;
}

//********************************************************************************
#pragma mark - intefaces realization IEncodedFrameConsumer
void VideoDecoder::onEncodedFrameDelivered(AVPacket &encodedImage,
                                           double timestamp,
                                           bool completeFrame)
{
    //if (frameCount_ == 0)
    {
        VLOG(LOG_TRACE)
        << getDescription()
        << " start decode "
        << frameCount_ << " "
        << encodedImage.size << " "
        << hex << (void*)encodedImage.data << dec
        //<< (encodedImage.getType() == webrtc::kKeyFrame ? "KEY" : "DELTA")
        << std::endl;
    }

    //NdnUtils::printMem("decoderMem", encodedImage.data, 20);

    capturedTimestamp_ = timestamp;
    frameCount_++;

    //VLOG(LOG_TRACE) << "decoding ... " << std::endl;
    int ret = decoder_->decode(encodedImage);

    VLOG_IF(LOG_WARN, ret < 0) << "decode ERROR errcode=" << ret << std::endl;
    VLOG_IF(LOG_WARN, ret == 0) << "no decoded frame ret=" << ret << std::endl;
}

int VideoDecoder::resetDecoder()
{
    frameCount_ = 0;

    if (!decoder_.get())
        return RESULT_ERR;

    decoder_->RegisterDecodeCompleteCallback(this);

    if (!(decoder_->InitDeocder(640,480/*settings_.encodeWidth_,settings_.encodeHeight_*/)))
        return RESULT_ERR;

    return RESULT_OK;
}
