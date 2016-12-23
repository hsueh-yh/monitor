#include <stdlib.h>

#include "simple-log.h"
#include "glogger.h"
#include "renderer.h"


uint8_t*
RendererInternal::getFrameBuffer(int width, int height)
{
    if (!renderBuffer_ || width*height*3/2 > bufferSize_)
    {
        // init RGB buffer
        bufferSize_ = width*height*3/2;
        VLOG(LOG_TRACE) << "initializing renderer buffer "
                        << width << "x" << height << "("
                        << bufferSize_ <<" bytes)..."
                        << std::endl;

        LogTrace(logFile_) << "initializing renderer buffer "
                             << width << "x" << height << "("
                             << bufferSize_ <<" bytes)..."
                             << std::endl;
        renderBuffer_ = (uint8_t*)realloc(renderBuffer_, bufferSize_);

        RGBbufSize_ = width*height*3;
        RGBbuf_ = (uint8_t*)realloc(RGBbuf_, RGBbufSize_);
        //image = new QImage(RGBbuf_,width,height,QImage::Format_RGB888);
    }
    //memset(renderBuffer_, 0, bufferSize_);//YUV420

    return (uint8_t*)renderBuffer_;
}

void
RendererInternal::renderYUVFrame(int64_t timestamp, int width, int height,
                     const uint8_t* buffer)
{
    // do whatever we need, i.e. drop frame, render it, write to file, etc.
    VLOG(LOG_TRACE) << "Rendering frame (" << width << "x" << height << ") at " << timestamp << "ms" << std::endl;
    LogTrace(logFile_) << "Rendering frame (" << width << "x" << height << ") at " << timestamp << "ms" << std::endl;

    YV12ToBGR24_Native(buffer,RGBbuf_,width,height);

    image = QImage(RGBbuf_,width,height,QImage::Format_RGB888);
    //image->loadFromData((const unsigned char*)RGBbuf_, (int)RGBbufSize_, "BMP");
    QPixmap pixmap = QPixmap(width,height);
    pixmap = QPixmap::fromImage(image.scaled(width_, height_/*, Qt::IgnoreAspectRatio*/));


    label_->setPixmap(pixmap);
    label_->resize(width_,height_);
    label_->move(point_);
    label_->show();

    //LOG(INFO) << "Rendering frame done (" << width << "x" << height << ") at " << timestamp << "ms" << std::endl;

    return;
}
