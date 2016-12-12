//#pragma once
#ifndef FFDECODER_H_
#define FFDECODER_H_

#include "tdll.h"

#include <iostream>

using namespace std;

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h" 
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libpostproc/postprocess.h"
#include "libavutil/pixfmt.h"
};

#include "renderer.h"

#define BYTE unsigned char


class DecodedImageCallback
{
public:
    virtual void onDecoded(const AVFrame &decodedFrame) = 0;
};


class Decoder /*: public IH264Decoder*/
{
public:
    Decoder(void);
    virtual ~Decoder(void);

    virtual bool InitDeocder(int width, int height);
    virtual bool setContext(uint8_t *sps, int spslen, uint8_t *pps, int ppslen );
    virtual bool decode(unsigned char *inbuf, const int  &inlen, unsigned char *outbuf, int  &outlen);
    virtual int decode(AVPacket &encodedImage);
	virtual void StopDecoder();
	virtual void ReleaseConnection();
    bool YV12ToBGR24_FFmpeg(unsigned char *pYUV,unsigned char *pBGR24,int width,int height);

    void
    RegisterDecodeCompleteCallback(DecodedImageCallback *callback)
    { callback_ = callback; }

private:

	bool LoadDllFun();
	//void(*avcodec_init)(void);
	void(*avcodec_register_all)(void);
	void(*av_init_packet)(AVPacket *pkt);
    AVCodecContext *(*avcodec_alloc_context3)(const AVCodec *codec);
    AVFrame *(*av_frame_alloc)(void);
    AVCodec *(*avcodec_find_decoder)(enum AVCodecID id);
	//int(*avcodec_decode_video)(AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr,
	//	uint8_t *buf, int buf_size);
    int (*avpicture_fill)(AVPicture *picture, const uint8_t *ptr,
                       enum AVPixelFormat pix_fmt, int width, int height);
    int(*avcodec_decode_video2)(AVCodecContext *avctx, AVFrame *picture,
                                int *got_picture_ptr, AVPacket *avpkt);
	int(*avcodec_open2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
	int(*avcodec_close)(AVCodecContext *avctx);
    void(*av_frame_unref)(AVFrame *frame);
    void(*av_frame_free)(AVFrame **frame);
    void(*av_free)(void *ptr);

	bool InitPostproc(int w, int h);
	void ClosePostproc();
	pp_context *(*pp_get_context)(int width, int height, int flags);
	void(*pp_free_context)(pp_context *ppContext);
	void(*pp_free_mode)(pp_mode *mode);
    pp_mode *(*pp_get_mode_by_name_and_quality)(char *name, int quality);
	///*
    void(*pp_postprocess)(uint8_t  *src[3], int srcStride[3],
        uint8_t  *dst[3], int dstStride[3],
		int horizontalSize, int verticalSize,
		QP_STORE_T *QP_store, int QP_stride,
		pp_mode *mode, pp_context *ppContext, int pict_type);
		//*/

    struct SwsContext *(*sws_getContext)(int srcW, int srcH, enum AVPixelFormat srcFormat,
                                      int dstW, int dstH, enum AVPixelFormat dstFormat,
                                      int flags, SwsFilter *srcFilter,
                                      SwsFilter *dstFilter, const double *param);
    int (*sws_scale)(struct SwsContext *c, const uint8_t *const srcSlice[],
                  const int srcStride[], int srcSliceY, int srcSliceH,
                  uint8_t *const dst[], const int dstStride[]);
    void (*sws_freeContext)(struct SwsContext *swsContext);


    void frame_rotate_180(AVFrame *src,AVFrame*des);

private:
    AVCodec			*pdecoder;
    AVCodecContext	*pdecContext;
    AVFrame			*pDecodedFrame;
	AVPacket		avpkt;
	int				m_width;
	int				m_height;

	Tdll *avcdll;
	Tdll *utildll;
    Tdll *prodll;
    Tdll *swsdll;
	pp_context *pp_context_;
    pp_mode    *pp_mode_;

    DecodedImageCallback *callback_;
};

#endif //FFDECODER_H_
