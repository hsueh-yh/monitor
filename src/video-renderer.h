#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include "renderer.h"
#include "interfaces.h"


class IVideoRenderer : public IRenderer, public IRawFrameConsumer
{
public:
    IVideoRenderer(){}
    virtual ~IVideoRenderer(){}
};


/**
  *This class is used for sending RGB buffers to the external renderer.
  *It conforms to the internal library interfaces (i.e. IRawFrameConsumer)
  *and can be used as a normal renderer. However, internally it does not
  *perform actual rendering - instead, it performs transcoding incoming
  *frames into RGB format and copies RGB data into buffer, provided by
  *external renderer.
 */
class ExternalVideoRendererAdaptor : public IVideoRenderer
{
public:
    ExternalVideoRendererAdaptor(IExternalRenderer *externalRenderer);
    virtual ~ExternalVideoRendererAdaptor(){}

    int
    init();

    int
    startRendering(const std::string &windowName = "Renderer");

    int
    stopRendering();

    void
    onDeliverFrame(const AVFrame &rawFrame, double timestamp);

private:
    IExternalRenderer *externalRenderer_;
};

#endif // VIDEORENDERER_H
