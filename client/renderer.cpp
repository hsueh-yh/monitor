#include <stdlib.h>

#include "logger.h"
#include "renderer.h"


uint8_t*
RendererInternal::getFrameBuffer(int width, int height)
{
    if (!renderBuffer_ || width*height*3 > bufferSize_)
    {
        // init RGB buffer
        bufferSize_ = width*height*3;
        LOG(INFO) << "initializing renderer buffer " << width << "x" << height << "(" << bufferSize_ <<" bytes)..." << std::endl;
        renderBuffer_ = (uint8_t*)realloc(renderBuffer_, bufferSize_);
    }

    return (uint8_t*)renderBuffer_;
}

void
RendererInternal::renderYUVFrame(int64_t timestamp, int width, int height,
                     const uint8_t* buffer)
{
    uint8_t* rgbbuf = new uint8_t[width  *height  *3];
    YV12ToBGR24_Native(buffer,rgbbuf,width,height);
    LOG(INFO) << "transcoding YUV420 to RGB " << std::endl;

    QImage *image = new QImage(rgbbuf,width,height,QImage::Format_RGB888);
    LOG(INFO) << "new QImage " << std::endl;
    QPixmap pixmap = QPixmap(width,height);
    pixmap = QPixmap::fromImage(image->scaled(width, height, Qt::KeepAspectRatio));
    LOG(INFO) << "QPixmap "
              << pixmap.width() << "*" << pixmap.height() << std::endl;

    if( label_->isEnabled())
        LOG(INFO) << "QLabel "
                  << label_->width() << "*" << label_->height() << std::endl;

    label_->setPixmap(pixmap);
    LOG(INFO) << "setPixmap " << std::endl;
    label_->resize(width_,height_);
    LOG(INFO) << "resize " << label_->width() << "*" << label_->height() << std::endl;
    label_->move(point_);
    label_->show();
    LOG(INFO) << "move " << std::endl;
    // do whatever we need, i.e. drop frame, render it, write to file, etc.
    LOG(INFO) << "Rendering frame (" << width << "x" << height << ") at " << timestamp << "ms" << std::endl;
    return;
}
