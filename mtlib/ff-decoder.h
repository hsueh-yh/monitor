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


class FF_DecodedImageCallback
{
public:
    virtual void onDecoded(const AVFrame &decodedFrame) = 0;
};


//////////////////////////////////////////////////////////////
/// \brief The Decoder class
/// Author:Vincent Luo
/// Date: 20150615
/// Description:参考H264标准语法实现对SPS参数的解析
//////////////////////////////////////////////////////////////

#include <iostream>
#include <math.h>
#include <string>

class H264SPSPaser {
public:
   //private static final string TAG = "H264SPSPaser";
   static int startBit;
   /*
    * 从数据流data中第StartBit位开始读，读bitCnt位，以无符号整形返回
    */
   static uint16_t u(uint8_t *data,int bitCnt,int StartBit){
       uint16_t ret = 0;
       int start = StartBit;
       for(int i = 0;i < bitCnt;i++){
           ret<<=1;
           if ((data[start / 8] & (0x80 >> (start%8))) != 0)
           {
               ret += 1;
           }
           start++;
       }
       return ret;
   }
   /*
    * 无符号指数哥伦布编码
    * leadingZeroBits = ?1;
    * for( b = 0; !b; leadingZeroBits++ )
    *    b = read_bits( 1 )
    * 变量codeNum 按照如下方式赋值：
    * codeNum = 2^leadingZeroBits ? 1 + read_bits( leadingZeroBits )
    * 这里read_bits( leadingZeroBits )的返回值使用高位在先的二进制无符号整数表示。
    */
   static uint16_t ue(uint8_t *data,int StartBit){
       uint16_t ret = 0;
       int leadingZeroBits = -1;
       int tempStartBit = (StartBit == -1)?startBit:StartBit;//如果传入-1，那么就用上次记录的静态变量
        for( int b = 0; b != 1; ++leadingZeroBits ){//读到第一个不为0的数，计算前面0的个数
            b = u(data,1,tempStartBit++);
        }
        //Log.d(TAG,"ue leadingZeroBits = " + leadingZeroBits + ",Math.pow(2, leadingZeroBits) = " + Math.pow(2, leadingZeroBits) + ",tempStartBit = " + tempStartBit);
        ret = (uint16_t) (pow(2, leadingZeroBits) - 1 + u(data,leadingZeroBits,tempStartBit));
        startBit = tempStartBit + leadingZeroBits;
        //Log.d(TAG,"ue startBit = " + startBit);
       return ret;
   }
   /*
    * 有符号指数哥伦布编码
    * 9.1.1 有符号指数哥伦布编码的映射过程
    *按照9.1节规定，本过程的输入是codeNum。
    *本过程的输出是se(v)的值。
    *表9-3中给出了分配给codeNum的语法元素值的规则，语法元素值按照绝对值的升序排列，负值按照其绝对
    *值参与排列，但列在绝对值相等的正值之后。
    *表 9-3－有符号指数哥伦布编码语法元素se(v)值与codeNum的对应
    *codeNum 语法元素值
    *  0       0
    *  1       1
    *  2       ?1
    *  3       2
    *  4       ?2
    *  5       3
    *  6       ?3
    *  k       (?1)^(k+1) Ceil( k÷2 )
    */
   static int se(uint8_t *data,int StartBit){
       int ret = 0;
       uint16_t codeNum = ue(data,StartBit);
       ret = (int) (pow(-1, codeNum + 1)*ceil(codeNum/2));
       return ret;
   }
};


class Decoder /*: public IH264Decoder*/
{
public:
    Decoder(void);
    virtual ~Decoder(void);

    virtual bool InitDeocder();
    virtual bool setContext(uint8_t *sps, int spslen, uint8_t *pps, int ppslen );
    virtual bool decode(unsigned char *inbuf, const int  &inlen, unsigned char *outbuf, int  &outlen);
    virtual int decode(AVPacket &encodedImage);
	virtual void StopDecoder();
	virtual void ReleaseConnection();
    bool YV12ToBGR24_FFmpeg(unsigned char *pYUV,unsigned char *pBGR24,int width,int height);

    void
    RegisterDecodeCompleteCallback(FF_DecodedImageCallback *callback)
    { callback_ = callback; }

    int wigth()
    {
        return m_width;
    }
    int height()
    {
        return m_height;
    }

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

    FF_DecodedImageCallback *callback_;
};

#endif //FFDECODER_H_
