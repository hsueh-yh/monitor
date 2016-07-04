/*
 * player.cpp
 *
 *  Created on: Jun 17, 2016
 *      Author: xyh
 */

#include "player.h"

static int FindStartCode(unsigned char *Buf, int zeros_in_startcode)
{
	int info;
	int i;

	info = 1;
	for (i = 0; i < zeros_in_startcode; i++)
	{
		if (Buf[i] != 0)
			info = 0;
	}

	if (Buf[i] != 1)
		info = 0;
	return info;
}

static bool Check_StartCode(unsigned char *Buf, int pos)
{
	int info3 = 0;

	info3 = FindStartCode(&Buf[pos - 4], 3);
	return info3 == 1;

}

static int getNextNal(unsigned char* &inpf, unsigned char* inpf_end, unsigned char* inBuf)
{
	int inBuf_pos = 0;
	int StartCodeFound = 0;
	int info2 = 0;
	int info3 = 0;

	int nCount = 0;
	while ( inpf <= inpf_end && ++nCount <= 4)
	{
		inBuf[inBuf_pos++] = *inpf++;
	}

	if (!Check_StartCode(inBuf, inBuf_pos))
	{
		return 0;
	}


	//while (!feof(inpf) && (inBuf[inBuf_pos++] = fgetc(inpf)) == 0);

	//find the next start code
	while (!StartCodeFound)
	{
		//end of file
		if (inpf>=inpf_end)
		{
			//			return -1;
			return inBuf_pos - 1;
		}
		inBuf[inBuf_pos++] = *inpf++;

		StartCodeFound = Check_StartCode(inBuf, inBuf_pos);
	}

	//fseek(inpf, -4, SEEK_CUR);
	inpf -= 4;

	// return the end(length) of this NALU
	return inBuf_pos - 4;
}



Player::Player():
    decoder_(new Decoder()),
    //label(new QLabel()),
    pixmap(new QPixmap()),
    status_(STOPED),
    yuv_frameBuf_(new unsigned char[640 * 480 * 3 / 2]),
    bmp_frameBuf_(new unsigned char[640 * 480 * 3/*1536000+54*/])

{
    pFile_ = fopen ( "playerout.yuv", "wb+" );
	if ( pFile_ == NULL )
	{
		std::cout << "open consumer.yuv error" << std::endl;
		return;
	}

	pFile1_ = fopen ( "testPlayer.264", "wb+" );
	if ( pFile1_ == NULL )
	{
		std::cout << "open consumer.yuv error" << std::endl;
		return;
	}

}

Player::~Player()
{
	fclose(pFile_);
	fclose(pFile1_);
	decoder_->StopDecoder();
	decoder_->ReleaseConnection();
}


bool Player::init (boost::shared_ptr<FrameBuffer> frameBuffer)
{
    cout << "Player: " << (int)getpid() << "-"
         << std::this_thread::get_id() << " ";
    frameBuffer_ = frameBuffer;

/*
    FrameBuffer::Slot *slot1 =NULL, *slot2 =NULL;
    uint8_t *sps, *pps;
    int spslen,ppslen;

    while ( slot1== NULL )
        slot1 = frameBuffer_->getFrame();

    sps = slot1->getDataPtr();
    spslen = slot1->getFrameSize();

    while ( slot2== NULL )
            slot2 = frameBuffer_->getFrame();

    pps = slot2->getDataPtr();
    ppslen = slot2->getFrameSize();

    cout << "spslen" << spslen << "  ppslen " << ppslen << endl;
*/

    if (!decoder_->InitDeocder(640, 480))
	{
		return false;
	}

//    decoder_->setContext(sps, spslen, pps, ppslen);
    status_ = READY;

	return true;
}


void Player::writeFile ()
{
    //init();
	//sleep(3);
	std::cout<< std::endl<< std::endl << " Write start " << std::endl<< std::endl;
	//cout << endl << "start write" << endl<< endl<< endl;
	int i=0;
	std::cout << i <<std::endl;
	//for ( int i = 0; i < 200; i++ )
	while( ++i <= 202)
	{

        //FrameBuffer::Slot *slot =NULL;
        boost::shared_ptr<FrameBuffer::Slot> slot;
		//cout << frameBuffer_->status_ << endl;
		//while(frameBuffer_->status_ != STARTED);

		while ( slot== NULL )
            slot = frameBuffer_->popSlot();

		unsigned char *p_Out_Frame = new unsigned char[640 * 480 * 3 / 2];
		unsigned char *p_In_Frame = slot->getDataPtr();
		int outlen, inlen;
        inlen = slot->getPayloadSize();

//		std::cout << "Write " << i << " " << "size:" << inlen<< std::endl;
//		cout << slot->getSlotNumber()<<endl;
		for( int i = 0; i <20; i++ )
				printf("%X ",p_In_Frame[i]);
		cout << endl;
//		std::cout << std::endl << std::endl;
//		fwrite ( p_In_Frame, inlen, 1, pFile1_ );

        std::cout << std::endl << "SizeIn: " << slot->getPayloadSize() << std::endl;

		decoder_->decode( p_In_Frame, inlen, p_Out_Frame, outlen );
		//decoder_->decode( pFile1_,slot->getDataPtr(),slot->getFrameSize(), p_Out_Frame, outlen );

		std::cout << std::endl << "SizeOut: " << outlen << std::endl;

		if ( outlen > 0 )
			fwrite ( p_Out_Frame, outlen, 1, pFile_ );

		usleep(300);
	}
	cout << endl << "wirte thread end" << endl;
}

static void YUV420p_to_RGB24(unsigned char *yuv420[3], unsigned char *rgb24, int width, int height)
{
    //  int begin = GetTickCount();
    int R,G,B,Y,U,V;
    int x,y;
    int nWidth = width>>1; //色度信号宽度
    for (y=0;y<height;y++)
    {
        for (x=0;x<width;x++)
        {
            Y = *(yuv420[0] + y*width + x);
            U = *(yuv420[1] + ((y>>1)*nWidth) + (x>>1));
            V = *(yuv420[2] + ((y>>1)*nWidth) + (x>>1));
            R = Y + 1.402*(V-128);
            G = Y - 0.34414*(U-128) - 0.71414*(V-128);
            B = Y + 1.772*(U-128);

            //防止越界
            if (R>255)R=255;
            if (R<0)R=0;
            if (G>255)G=255;
            if (G<0)G=0;
            if (B>255)B=255;
            if (B<0)B=0;

            *(rgb24 + ((height-y-1)*width + x)*3) = B;
            *(rgb24 + ((height-y-1)*width + x)*3 + 1) = G;
            *(rgb24 + ((height-y-1)*width + x)*3 + 2) = R;
            //    *(rgb24 + (y*width + x)*3) = B;
            //    *(rgb24 + (y*width + x)*3 + 1) = G;
            //    *(rgb24 + (y*width + x)*3 + 2) = R;
        }
    }
}

unsigned char* Player::imageProcess(const void* p,unsigned char* dst)
{
        unsigned char* src = (unsigned char*)p;

        unsigned char bmp_h[] =
        {
                        0x42,0x4d,            //BM
                        0x42,0x70,0x17,0x00,  // 172000+66
                        0x00,0x00,0x00,0x00,
                        0x36,0x00,0x00,0x00,  //bmp_data offset
                        0x28,0x00,0x00,0x00,
                        //0x20,0x03,0x00,0x00,   //width
                        0x80,0x02,0x00,0x00,   //width
                        0x20,0xfe,0xff,0xff,   //hieght
                        0x01,0x00,
                        0x20,0x00,             //32bit
                        0x00,0x00,0x00,0x00,
                        0x00,0x70,0x17,0x00,  //bmp_data size
                        0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00
        };

        memcpy(dst,bmp_h,54);
        memcpy(dst+54,src,640*480*3/2);

        return dst;

}


unsigned char* Player::getPixmap()
{
    //init();
    //sleep(3);
    if(status_!=READY)
    {
        cout << "Player not ready" << endl;
        return NULL;
    }

    int i=0;

    //std::cout<< std::endl<< std::endl << "...... Get frame start ...... " << std::endl;

    while( ++i <= 202)
    {
        boost::shared_ptr<FrameBuffer::Slot> slot;

        while ( slot== NULL )
        {
            slot = frameBuffer_->popSlot();
            usleep(100);
        }

        //unsigned char *p_Out_Frame = new unsigned char[640 * 480 * 3 / 2];
        unsigned char *p_In_Frame = slot->getDataPtr();
        int outlen, inlen;
        inlen = slot->getPayloadSize();

/*
        for( int i = 0; i <20; i++ )
                printf("%2X ",p_In_Frame[i]);
        cout << endl;
*/



        decoder_->decode( p_In_Frame, inlen, yuv_frameBuf_, outlen );
        //decoder_->decode( pFile1_,slot->getDataPtr(),slot->getFrameSize(), p_Out_Frame, outlen );

//        std::cout << std::endl << "SizeIn: " << slot->getPayloadSize()
//                  << " SizeOut: " << outlen << std::endl;



        if ( outlen > 0 )
        {
            cout << slot->getNumber() << " "
                 << slot->getPayloadSize() << " ";
//            cout << slot->getNumber()
//                << "------------------------got yuv frame---------------------------" << endl;
            //decoder_->YV12ToBGR24_FFmpeg(p_Out_Frame,image_buf,640,480);

            //imageProcess(yuv_frameBuf_, bmp_frameBuf_);
            unsigned char* yuv[3] = {yuv_frameBuf_,yuv_frameBuf_+ 640*480, yuv_frameBuf_ +640*480*5/4};

            YUV420p_to_RGB24(yuv,bmp_frameBuf_,640,480);
            fwrite ( bmp_frameBuf_, 640*480*3/*outlen+54*/, 1, pFile_ );

            return NULL;
            //return bmp_frameBuf_;
            //pixmap->loadFromData(image_buf, 800*480*4+54, "bmp", NULL);
        }
        cout << endl << slot->getNumber() << " " << slot->getPayloadSize() << endl << endl;
        //return pixmap;

        usleep(100);
        //return NULL;

    }
    //cout << endl << "wirte thread end" << endl;
}
