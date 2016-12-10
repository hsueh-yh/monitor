
#include "video-renderer.h"
#include "utils.h"
#include "defines.h"


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
    uint8_t *rgbFrameBuffer = externalRenderer_->getFrameBuffer(rawFrame.width,
                                                                rawFrame.height);
    if (rgbFrameBuffer)
    {
        //ConvertFromI420(frame, kBGRA, 0, rgbFrameBuffer);
        externalRenderer_->renderYUVFrame(NdnUtils::millisecondTimestamp(),
                                          rawFrame.width, rawFrame.height,
                                          rgbFrameBuffer);
    }
}
