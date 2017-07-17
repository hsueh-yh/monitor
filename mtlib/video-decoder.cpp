#include "video-decoder.h"
#include "mtndn-defines.h"
#include "glogger.h"
#include "mtndn-utils.h"


//********************************************************************************
#pragma mark - construction/destruction
VideoDecoder::VideoDecoder() :
    decodedFrameConsumer_(NULL)
{
    decoder_.reset(new Decoder());
    std::stringstream ss;
    ss << componentId_;
    setDescription(ss.str());
    setDescription("[VideoDecoder]");
    LogInfoC << getDescription() << std::endl;
}

//********************************************************************************
#pragma mark - public
int VideoDecoder::init(ndnlog::new_api::Logger *logger)
{
    //settings_ = settings;
    if (RESULT_GOOD(resetDecoder()))
        return RESULT_OK;

    setLogger(logger);

    return RESULT_ERR;
}

void VideoDecoder::reset()
{
    if (frameCount_ > 0)
    {
        LogTraceC << "resetting decoder..." << std::endl;
        //int res = decoder_->Release();
        resetDecoder();
        LogTraceC << "decoder reset" << std::endl;
    }
}

//********************************************************************************
#pragma mark - intefaces realization DecodedImageCallback
void VideoDecoder::onDecoded(const AVFrame &decodedImage)
{
    /*
    LogTraceC << "decode Frame "
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
        LogTraceC
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

    if( ret < 0 )
    LogWarnC << "decode ERROR errcode=" << ret << std::endl;
    if( ret == 0 )
        LogWarnC << "no decoded frame ret=" << ret << std::endl;
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
