
#include "video-renderer.h"
#include "mtndn-utils.h"
#include "mtndn-defines.h"


ExternalVideoRendererAdaptor::ExternalVideoRendererAdaptor
                        (IExternalRenderer *externalRenderer):
    externalRenderer_(externalRenderer)
{
    //description_ = "external-renderer";
}

int
ExternalVideoRendererAdaptor::init()
{
    return RESULT_OK;
}

int
ExternalVideoRendererAdaptor::startRendering(const std::string &windowName)
{
    isRendering_ = true;
    return RESULT_OK;
}

int
ExternalVideoRendererAdaptor::stopRendering()
{
    isRendering_ = false;
    return RESULT_OK;
}

//******************************************************************************
void
ExternalVideoRendererAdaptor::onDeliverFrame(const AVFrame &rawFrame,
                                             double timestamp)
{
    uint8_t *YUVFrameBuffer = externalRenderer_->getFrameBuffer(rawFrame.width,
                                                                rawFrame.height);

    //memset(outbuf, 0, rawFrame.height * rawFrame.width * 3 / 2);//YUV420

    int a = 0, i;
    for (i = 0; i<rawFrame.height; i++)
    {
        memcpy(YUVFrameBuffer + a, rawFrame.data[0] + i * rawFrame.linesize[0], rawFrame.width);
        a += rawFrame.width;
    }
    for (i = 0; i<rawFrame.height / 2; i++)
    {
        memcpy(YUVFrameBuffer + a, rawFrame.data[1] + i * rawFrame.linesize[1], rawFrame.width / 2);
        a += rawFrame.width / 2;
    }
    for (i = 0; i<rawFrame.height / 2; i++)
    {
        memcpy(YUVFrameBuffer + a, rawFrame.data[2] + i * rawFrame.linesize[2], rawFrame.width / 2);
        a += rawFrame.width / 2;
    }

    if (YUVFrameBuffer)
    {
        //ConvertFromI420(frame, kBGRA, 0, rgbFrameBuffer);
        externalRenderer_->renderYUVFrame(timestamp/*MtNdnUtils::millisecondTimestamp()*/,
                                          rawFrame.width, rawFrame.height,
                                          YUVFrameBuffer);
    }
}
