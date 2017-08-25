
#include "ff-decoder.h"
#include "glogger.h"
#include "mtndn-utils.h"

int H264SPSPaser::startBit = 0;


Decoder::Decoder(void)
    : pdecoder(NULL)
	, pdecContext(NULL)
    , pDecodedFrame(NULL)
	, pp_context_(NULL)
	, pp_mode_(NULL)
    , avcdll(NULL)
	, prodll(NULL)
	, utildll(NULL)
    , swsdll(NULL)
    , callback_(nullptr)
    , m_width(-1)
    , m_height(-1)
{}

Decoder::~Decoder(void)
{
    StopDecoder();
}

bool Decoder::LoadDllFun()
{
    const char* avcode = "libavcodec.so";
    const char* util = "libavutil.so";
    const char* postpro = "libpostproc.so";
    const char* swscale = "libswscale.so";
	///*
	avcdll = new Tdll(avcode);
	avcdll->loadFunction((void**)&avcodec_register_all, "avcodec_register_all");
	avcdll->loadFunction((void**)&av_init_packet, "av_init_packet");
	avcdll->loadFunction((void**)&avcodec_alloc_context3, "avcodec_alloc_context3");
	avcdll->loadFunction((void**)&avcodec_find_decoder, "avcodec_find_decoder");
    avcdll->loadFunction((void**)&avpicture_fill, "avpicture_fill");
	avcdll->loadFunction((void**)&avcodec_open2, "avcodec_open2");
	avcdll->loadFunction((void**)&avcodec_decode_video2, "avcodec_decode_video2");
	avcdll->loadFunction((void**)&avcodec_close, "avcodec_close");
	//avcdll->loadFunction((void**)&av_free, "av_free");
	if (!avcdll->ok)
	{
		cout << "load avcodec dll error" << endl;
		return false;
	}

	utildll = new Tdll(util);
	utildll->loadFunction((void**)&av_frame_alloc, "av_frame_alloc");
    utildll->loadFunction((void**)&av_free, "av_free");
    utildll->loadFunction((void**)&av_frame_unref, "av_frame_unref");
    utildll->loadFunction((void**)&av_frame_free, "av_frame_free");
	if (!utildll->ok)
	{
		cout << "load util dll error" << endl;
		return false;
	}
///*
	prodll = new Tdll(postpro);
	prodll->loadFunction((void**)&pp_get_context, "pp_get_context");
	prodll->loadFunction((void**)&pp_free_context, "pp_free_context");
	prodll->loadFunction((void**)&pp_free_mode, "pp_free_mode");
	prodll->loadFunction((void**)&pp_get_mode_by_name_and_quality, "pp_get_mode_by_name_and_quality");
	prodll->loadFunction((void**)&pp_postprocess, "pp_postprocess");
	if (!prodll->ok)
	{
		cout << "load prodll error" << endl;
		return false;
	}
//*/
    swsdll = new Tdll(swscale);
    swsdll->loadFunction((void**)&sws_getContext, "sws_getContext");
    swsdll->loadFunction((void**)&sws_scale, "sws_scale");
    swsdll->loadFunction((void**)&sws_freeContext, "sws_freeContext");
    if (!swsdll->ok)
    {
        cout << "load swsdll error" << endl;
        return false;
    }

	return true;
}

bool Decoder::InitDeocder()
{
	if (!LoadDllFun())
		return false;
	
	//avcodec_init();
    //debug//cout << "avcodec_register_all..." << endl;
	avcodec_register_all();
    //debug//cout << "done." << endl;
	//if (!InitPostproc(width, height))
	//	return false;

    //debug//cout << "av_init_packet..." << endl;
	av_init_packet(&avpkt);

    //m_width = width;
    //m_height = height;
    //debug//cout << "avcodec_find_decoder..." << endl;
    pdecoder = avcodec_find_decoder(AV_CODEC_ID_H264);
	
    if (pdecoder == NULL)
		return false;

    pdecContext = avcodec_alloc_context3(pdecoder);

    //if 1, returned reference belongs to the caller
    //if 0, belongs to the decoder
    pdecContext->refcounted_frames = 0;
    pDecodedFrame = av_frame_alloc();

    //pdecContext->width = width;
    //pdecContext->height = height;
    //pdecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	
    /* open it */
    //debug//cout << "avcodec_open2" << endl;
    if (avcodec_open2(pdecContext, pdecoder, NULL) < 0)
	{
		cout << "avcodec_open2 error!" << endl;
		return false;
	}
	return true;
}

bool Decoder::InitPostproc(int w, int h)
{
	///*
    //debug//cout << "InitPostproc" << endl;
	int i_flags = 0;
	i_flags = PP_CPU_CAPS_MMX | 
				PP_CPU_CAPS_MMX2 | 
				PP_FORMAT_420;
	
	pp_context_ = pp_get_context(w, h, i_flags);
	
	if (!pp_context_)
		return false;
	
	pp_mode_ = pp_get_mode_by_name_and_quality("default", 6);
	
	if (!pp_mode_)
		return false;
	//*/
	return true;
}

bool Decoder::setContext(uint8_t *sps, int spslen, uint8_t *pps, int ppslen )
{
    pdecContext->extradata = new uint8_t[spslen+ppslen];
    pdecContext->extradata_size = spslen+ppslen;

    memcpy(pdecContext->extradata, sps, spslen);
    memcpy(pdecContext->extradata+spslen, pps, ppslen);
}

int Decoder::decode( AVPacket &encodedImage )
{
    int got_frame, ret;
    av_init_packet(&encodedImage);

    //NdnUtils::printMem("ffdecoderMem", encodedImage.data, 20);

    ret = avcodec_decode_video2(pdecContext, pDecodedFrame, &got_frame, &encodedImage);

    if ( got_frame!=0 )
    {
        /*
        LOG(INFO) << "AVFrame: "
                  << pDecodedFrame->linesize[0] << " "
                  << pDecodedFrame->linesize[1] << " "
                  << pDecodedFrame->linesize[2] << " "
                  << std::endl;
        */
        m_width= pDecodedFrame->width;
        m_height = pDecodedFrame->height;

        if( callback_ != nullptr )
            callback_->onDecoded(*pDecodedFrame);
    }

    return ret;
}

static bool isSPS( unsigned char *buf )
{
    return (buf[0] == 0 && buf[1] == 0 && buf[2] == 0
                && buf[3] == 0x01 && buf[4] == 0x67);
}

bool Decoder::decode( unsigned char * inbuf, const int  &inlen, unsigned char * outbuf, int  &outlen)
{
    if( m_width == -1 || m_height == -1 )
    {
        MtNdnUtils::printMem("SPS", inbuf, 8 );
        if( !isSPS(inbuf) )
            return false;
        int width = (H264SPSPaser::ue(inbuf,34) + 1)*16;
        int height = (H264SPSPaser::ue(inbuf,-1) + 1)*16;
        cout << width << "  " << height << endl;
        if( width <= 0 || height <= 0 )
            return false;
        m_width = width;
        m_height = height;
    }
    outlen = 0;
    int got_frame;

	int len;
	avpkt.size = inlen;
    avpkt.data = inbuf;

    /*
    cout << "avpkt:" << inlen << " " << strlen((char*)avpkt.data) << endl;

    cout << endl;
    for( int i = 0; i <20; i++ )
        printf("%X ",inbuf[i]);
    std::cout << std::endl << std::endl;

    cout << pdecContext->extradata_size << endl;
    cout <<endl<<endl<<endl;
    for ( int i = 0; i < pdecContext->extradata_size; i++ )
        printf("%X ", pdecContext->extradata[i]);
    cout <<endl<<endl<<endl;
    */


    len = avcodec_decode_video2(pdecContext, pDecodedFrame, &got_frame, &avpkt);
/*
    cout    << endl
            << "width " << pdecContext->width << endl
            << "height: " << pdecContext->height << endl
            << "bit_rate " << pdecContext->bit_rate  << endl
            << "framerate " << pdecContext->framerate.num<< "/"<<pdecContext->framerate.den << endl
            << "frame_bits " << pdecContext->frame_bits << endl
            << "frame_number " << pdecContext->frame_number << endl
            << "frame_size " << pdecContext->frame_size << endl
            << "gop_size " << pdecContext->gop_size << endl
            << "has_b_frames " << pdecContext->has_b_frames << endl
            << endl;
*/
    if (len < 0)
	{
        //debug//cout << "decode error ";
		return false;
	}
    //debug//cout << "got_frame:" << got_frame << endl;
	if (got_frame)
    {
        memset(outbuf, 0, m_height * m_width * 3 / 2);//YUV420

        int a = 0, i;
        for (i = 0; i<m_height; i++)
        {
            memcpy(outbuf + a, pDecodedFrame->data[0] + i * pDecodedFrame->linesize[0], m_width);
            a += m_width;
        }
        for (i = 0; i<m_height / 2; i++)
        {
            memcpy(outbuf + a, pDecodedFrame->data[1] + i * pDecodedFrame->linesize[1], m_width / 2);
            a += m_width / 2;
        }
        for (i = 0; i<m_height / 2; i++)
        {
            memcpy(outbuf + a, pDecodedFrame->data[2] + i * pDecodedFrame->linesize[2], m_width / 2);
            a += m_width / 2;
        }

        outlen = m_width*m_height * 3 / 2;
    }
	else
	{
		outlen = 0;
        return false;
	}

	return true;
}

void Decoder::StopDecoder()
{
    if (pdecContext->refcounted_frames != 0 && pDecodedFrame)
    {
        av_frame_unref(pDecodedFrame);
        av_frame_free(&pDecodedFrame);
        //av_free(pDecodedFrame);
        //pDecodedFrame = NULL;
    }
	if (pdecContext != NULL)
    {
		avcodec_close(pdecContext);
		av_free(pdecContext);
        pdecContext = NULL;
	}
	if (avcdll) {
		delete avcdll;
		avcdll = 0;
	}

	ClosePostproc();
}

void Decoder::ClosePostproc()
{
	///*
	if (pp_mode_) {
		pp_free_mode(pp_mode_);
		pp_mode_ = 0;
	}
	if (pp_context_) {
		pp_free_context(pp_context_);
		pp_context_ = 0;
	}
    if (avcdll) {
        delete avcdll;
        avcdll = 0;
	}
    if (utildll) {
        delete utildll;
        utildll = 0;
    }
    if (prodll) {
        delete prodll;
        prodll = 0;
    }
    if (swsdll) {
        delete swsdll;
        swsdll = 0;
    }
    //*/
}

void Decoder::ReleaseConnection()
{
	delete this;
}


bool Decoder::YV12ToBGR24_FFmpeg(unsigned char* pYUV,unsigned char* pBGR24,int width,int height)
{
    if (width < 1 || height < 1 || pYUV == NULL || pBGR24 == NULL)
        return false;
    //int srcNumBytes,dstNumBytes;
    //uint8_t *pSrc,*pDst;
    AVPicture pFrameYUV,pFrameBGR;

    //pFrameYUV = avpicture_alloc();
    //srcNumBytes = avpicture_get_size(PIX_FMT_YUV420P,width,height);
    //pSrc = (uint8_t *)malloc(sizeof(uint8_t) * srcNumBytes);
    //av_image_fill_arrays(&pFrameYUV,width*height*3/2,pYUV,AV_PIX_FMT_YUV420P,width,height);
    avpicture_fill(&pFrameYUV,pYUV,AV_PIX_FMT_YUV420P,width,height);

    //U,V互换
    uint8_t * ptmp=pFrameYUV.data[1];
    pFrameYUV.data[1]=pFrameYUV.data[2];
    pFrameYUV.data [2]=ptmp;

    //pFrameBGR = avcodec_alloc_frame();
    //dstNumBytes = avpicture_get_size(PIX_FMT_BGR24,width,height);
    //pDst = (uint8_t *)malloc(sizeof(uint8_t) * dstNumBytes);
    avpicture_fill(&pFrameBGR,pBGR24,AV_PIX_FMT_RGB24,width,height);

    struct SwsContext* imgCtx = NULL;
    imgCtx = sws_getContext(width,height,AV_PIX_FMT_YUV420P,width,height,AV_PIX_FMT_BGR24,SWS_BILINEAR,0,0,0);

    if (imgCtx != NULL){
        sws_scale(imgCtx,pFrameYUV.data,pFrameYUV.linesize,0,height,pFrameBGR.data,pFrameBGR.linesize);
        if(imgCtx){
            sws_freeContext(imgCtx);
            imgCtx = NULL;
        }
        return true;
    }
    else{
        sws_freeContext(imgCtx);
        imgCtx = NULL;
        return false;
    }
}

void Decoder::frame_rotate_180(AVFrame *src,AVFrame*des)
{
    int n = 0,i= 0,j = 0;
    int hw = src->width>>1;
    int hh = src->height>>1;
    int pos= src->width * src->height;

    for (i = 0; i < src->height; i++)
    {
        pos-= src->width;
        for (int j = 0; j < src->width; j++) {
            des->data[0][n++] = src->data[0][pos + j];
        }
    }

    n = 0;
    pos = src->width * src->height>>2;

    for (i = 0; i < hh;i++) {
        pos-= hw;
        for (int j = 0; j < hw;j++) {

            des->data[1][n] = src->data[1][ pos + j];
            des->data[2][n] = src->data[2][ pos + j];
            n++;
        }
    }

    des->linesize[0] = src->width;
    des->linesize[1] = src->width>>1;
    des->linesize[2] = src->width>>1;

    des->width = src->width;
    des->height = src->height;
    des->format = src->format;

    des->pts = src->pts;
    des->pkt_pts = src->pkt_pts;
    des->pkt_dts = src->pkt_dts;

    des->key_frame = src->key_frame;
}
