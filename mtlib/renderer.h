#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include "frame-data.h"
#include "libavutil/frame.h"


class IRenderer
{
public:
    virtual ~IRenderer(){}
    virtual int init() = 0;
    virtual int startRendering(const std::string &name = "Renderer") = 0;
    virtual int stopRendering() = 0;

    bool
    isRendering()
    { return isRendering_; }

protected:
    bool isRendering_ = false;
};


class IRawFrameConsumer
{
public:
    virtual void onDeliverFrame(const AVFrame &frame,
                                double unixTimeStamp) = 0;
};


#endif // RENDERER_H
