#include "video-decoder.h"
#include "defines.h"
#include "logger.h"
#include "utils.h"


//********************************************************************************
#pragma mark - construction/destruction
VideoDecoder::VideoDecoder() :
    decodedFrameConsumer_(NULL),
    decoder_()
{
    decoder_.reset(new Decoder());
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
        LOG(INFO) << "resetting decoder..." << std::endl;
        //int res = decoder_->Release();
        resetDecoder();
        LOG(INFO) << "decoder reset" << std::endl;
    }
}

//********************************************************************************
#pragma mark - intefaces realization DecodedImageCallback
void VideoDecoder::onDecoded(const AVFrame &decodedImage)
{
    LOG(INFO) << "onDecoded "<< std::endl;
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
    if (frameCount_ == 0)
    {
        LOG(INFO)
        << "start decode "
        << frameCount_ << " "
        << encodedImage.size << " "
        << hex << (void*)encodedImage.data << dec
        //<< (encodedImage.getType() == webrtc::kKeyFrame ? "KEY" : "DELTA")
        << std::endl;
    }

    //NdnUtils::printMem("decoderMem", encodedImage.data, 20);

    capturedTimestamp_ = timestamp;
    frameCount_++;

    int ret = decoder_->decode(encodedImage);
    if( ret > 0 )
    {
        LOG(INFO) << "decoded " << frameCount_ << std::endl;
    }
    else if ( ret = 0 )
    {
        LOG(INFO) << "no decoded frame " << endl;

        //notifyError(-1, "can't decode frame");
    }
    else
        LOG(WARNING) << "decode ERROR " << ret << endl;

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
