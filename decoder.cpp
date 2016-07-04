
#include "decoder.h"

Decoder::Decoder(void)
    : pdecoder(NULL)
	, pdecContext(NULL)
	, pdecFrame(NULL)
	, pp_context_(NULL)
	, pp_mode_(NULL)
    , avcdll(NULL)
	, prodll(NULL)
	, utildll(NULL)
    , swsdll(NULL)
{
}

Decoder::~Decoder(void)
{
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

bool Decoder::InitDeocder(int width, int height)
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

	m_width = width;
	m_height = height;
    //debug//cout << "avcodec_find_decoder..." << endl;
    pdecoder = avcodec_find_decoder(AV_CODEC_ID_H264);
	
    if (pdecoder == NULL)
		return false;

    pdecContext = avcodec_alloc_context3(pdecoder);
	pdecFrame = av_frame_alloc();

	pdecContext->width = width;
	pdecContext->height = height;
	pdecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	
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

bool Decoder::decode( unsigned char * inbuf, const int & inlen, unsigned char * outbuf, int & outlen)
{
    outlen = 0;
	int got_frame;
	BYTE* showImage[3];
	int showheight[3], showLx[3];

	int len;
	avpkt.size = inlen;
	avpkt.data = inbuf;
	///cout << "avpkt:" << inlen << " " << strlen((char*)avpkt.data) << endl;

//	cout << endl;
//	for( int i = 0; i <20; i++ )
//			printf("%X ",inbuf[i]);
//	std::cout << std::endl << std::endl;

//	cout << pdecContext->extradata_size << endl;
//	cout <<endl<<endl<<endl;
//	for ( int i = 0; i < pdecContext->extradata_size; i++ )
//		printf("%X ", pdecContext->extradata[i]);
//	cout <<endl<<endl<<endl;

	len = avcodec_decode_video2(pdecContext, pdecFrame, &got_frame, &avpkt);
	if (len < 0)
	{
        //debug//cout << "decode error ";
		return false;
	}
    //debug//cout << "got_frame:" << got_frame << endl;
	if (got_frame)
	{
        memset(outbuf, 0, m_height * m_width * 3 / 2);

        int a = 0, i;
        for (i = 0; i<m_height; i++)
        {
            memcpy(outbuf + a, pdecFrame->data[0] + i * pdecFrame->linesize[0], m_width);
            a += m_width;
        }
        for (i = 0; i<m_height / 2; i++)
        {
            memcpy(outbuf + a, pdecFrame->data[1] + i * pdecFrame->linesize[1], m_width / 2);
            a += m_width / 2;
        }
        for (i = 0; i<m_height / 2; i++)
        {
            memcpy(outbuf + a, pdecFrame->data[2] + i * pdecFrame->linesize[2], m_width / 2);
            a += m_width / 2;
        }

        outlen = m_width*m_height * 3 / 2;

/*
		showImage[0] = outbuf;
		showImage[1] = showImage[0] + m_width*m_height;
		showImage[2] = showImage[1] + m_width*m_height / 4;
		showLx[0] = m_width; showLx[1] = m_width >> 1; showLx[2] = m_width >> 1;
		showheight[0] = m_height; showheight[1] = m_height >> 1; showheight[2] = m_height >> 1;
        //debug//cout << "pp_postprocess" <<endl;
        pp_postprocess(pdecFrame->data, pdecFrame->linesize, showImage, showLx, m_width, m_height, pdecFrame->qscale_table,
            pdecFrame->qstride, pp_mode_, pp_context_, pdecFrame->pict_type);
        GetImage(	pdecFrame->data,
                    showImage,
                    pdecFrame->linesize,
                    showLx,
                    showheight);
        debug//cout << "pp_postprocess done" <<endl;
		outlen = m_width*m_height * 3 / 2;

        cout << "pdecFrame:" << pdecFrame->linesize << " " << pdecFrame->width << "*" << pdecFrame->height << endl;

        memcpy(outbuf,pdecFrame->data,outlen);
        cout << "done" << endl;
*/
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
	if (pdecContext != NULL)
	{
		avcodec_close(pdecContext);
		av_free(pdecContext);
		pdecContext = NULL;
		if (pdecFrame) {
			av_free(pdecFrame);
			pdecFrame = NULL;
		}
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
	if (prodll) {
		delete prodll;
		prodll = 0;
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
