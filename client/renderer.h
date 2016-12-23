#ifndef RENDERER_H_
#define RENDERER_H_

#include "interfaces.h"
#include <sstream>

#include <QLabel>
static int logIdx = 0;

class RendererInternal : public IExternalRenderer{
public:
    RendererInternal(QWidget *parent):
        renderBuffer_(nullptr),
        bufferSize_(0),
        RGBbuf_(nullptr),
        RGBbufSize_(0),
        parent_(parent),
        label_(new QLabel(parent_)),
        point_(0,0),
        width_(0),height_(0)
    {
        logFile_ = "renderer";
        std::stringstream ss;
        ss << logIdx;
        ++logIdx;
        logFile_.append(ss.str());
        logFile_.append(".log");
        std::cout << "renderer log to " << logFile_ << std::endl;
    }

    ~RendererInternal()
    {
        if( label_ )
            delete label_;
        if( renderBuffer_ )
            free(renderBuffer_);
        if( RGBbuf_ )
            free (RGBbuf_);
    }


    void refresh(QWidget *parent, QPoint point, int width, int height)
    {
        parent_ = parent;
        point_ = point;
        width_ = width;
        height_ = height;
    }

    /**
      *Should return allocated buffer big enough to store RGB frame data
      *(width*height*3) bytes.
      *@param width Width of the frame (NOTE: width can change during run)
      *@param height Height of the frame (NOTE: height can change during run)
      *@return Allocated buffer where library can copy RGB frame data
     */
    virtual uint8_t* getFrameBuffer(int width, int height);

    /**
      *This method is called every time new frame is available for rendering.
      *This method is called on the same thread as getFrameBuffer was called.
      *@param timestamp Frame's timestamp
      *@param width Frame's width (NOTE: width can change during run)
      *@param height Frame's height (NOTE: height can change during run)
      *@param buffer Buffer with the RGB frame data (the same that was
      *returned from getFrameBuffer call)
      *@see getFrameBuffer
     */
    virtual void renderYUVFrame(int64_t timestamp, int width, int height,
                         const uint8_t* buffer);

private:
    uint8_t *renderBuffer_;
    std::size_t bufferSize_;

    uint8_t *RGBbuf_;
    std::size_t RGBbufSize_;

    QWidget *parent_;
    QLabel *label_;
    QPoint point_;
    int width_, height_;

    QImage image;
    QPixmap pixmap;

    std::string logFile_;

    bool
    YV12ToBGR24_Native(const uint8_t *pYUV,uint8_t *pBGR24,int width,int height)
    {
        if (width < 1 || height < 1 || pYUV == NULL || pBGR24 == NULL)
            return false;
        const long len = width  *height;
        const uint8_t *yData = pYUV;
        const uint8_t *vData = &yData[len];
        const uint8_t *uData = &vData[len >> 2];

        int bgr[3];
        int yIdx,uIdx,vIdx,idx;
        for (int i = 0;i < height;i++){
            for (int j = 0;j < width;j++){
                yIdx = i  *width + j;
                vIdx = (i/2)  *(width/2) + (j/2);
                uIdx = vIdx;

                bgr[0] = (int)(yData[yIdx] + 1.732446  *(uData[vIdx] - 128));                                    // b分量
                bgr[1] = (int)(yData[yIdx] - 0.698001  *(uData[uIdx] - 128) - 0.703125  *(vData[vIdx] - 128));    // g分量
                bgr[2] = (int)(yData[yIdx] + 1.370705  *(vData[uIdx] - 128));                                    // r分量

                for (int k = 0;k < 3;k++){
                    idx = (i  *width + j)  *3 + k;
                    if(bgr[k] >= 0 &&bgr[k] <= 255)
                        pBGR24[idx] = bgr[k];
                    else
                        pBGR24[idx] = (bgr[k] < 0)?0:255;
                }
            }
        }
        return true;
    }
};

#endif
