/*
 * player.cpp
 *
 *  Created on: Jun 17, 2016
 *      Author: xyh
 */

#include "player.h"

static int
FindStartCode(unsigned char *Buf, int zeros_in_startcode)
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


static bool
Check_StartCode(unsigned char *Buf, int pos)
{
	int info3 = 0;

	info3 = FindStartCode(&Buf[pos - 4], 3);
	return info3 == 1;

}


static int
getNextNal(unsigned char* &inpf, unsigned char* inpf_end, unsigned char* inBuf)
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


static void
YUV420p_to_RGB24(unsigned char *yuv420[3], unsigned char *rgb24, int width, int height)
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



Player::Player():
    decoder_(new Decoder()),
    yuv_frameBuf_(new unsigned char[WIDTH * HEIGHT * 3 / 2]),
    bmp_frameBuf_(new unsigned char[WIDTH * HEIGHT * 3/*1536000+54*/]),
    state_(Ready)

{
//    pFile_ = fopen ( "playerout.yuv", "wb+" );
//    if ( pFile_ == NULL )
//    {
//        std::cout << "open consumer.yuv error" << std::endl;
//        return;
//    }
}


Player::~Player()
{
    //fclose(pFile_);
    //decoder_->StopDecoder();
    //decoder_->ReleaseConnection();
    //frameBuffer_.reset();
#ifdef __SHOW_CONSOLE_
    cout << "[Player] dtor" << endl;
#endif
}


bool
Player::init (boost::shared_ptr<FrameBuffer> frameBuffer)
{
//    cout << "Player: " << (int)getpid() << "-"
//         << std::this_thread::get_id() << " ";
    frameBuffer_ = frameBuffer;

    if (!decoder_->InitDeocder(WIDTH, HEIGHT))
	{
		return false;
    }

    changetoState(Started);
	return true;
}


void
Player::start()
{
    changetoState(Started);
    while(1)
    {
        refresh();
        usleep(40*1000);
    }
}


void
Player::stop()
{
    lock();
    //decoder_->StopDecoder();
    //decoder_->ReleaseConnection();
    while(getState() != Stoped)
    {
        changetoState(Stoped);
    }
    unlock();
#ifdef __SHOW_CONSOLE_
    cout << "[Player] Stoping" << endl;
#endif
    return;
}


void
Player::changetoState(Player::State stat)
{
    //lock();
    state_ = stat;
    //unlock();
}


void
Player::writeFile ()
{
    std::cout<< std::endl<< std::endl << " Write start " << std::endl<< std::endl;
	int i=0;
    std::cout << i <<std::endl;
	while( ++i <= 202)
	{

        //FrameBuffer::Slot *slot =NULL;
        boost::shared_ptr<FrameBuffer::Slot> slot;
		//cout << frameBuffer_->status_ << endl;
		//while(frameBuffer_->status_ != STARTED);

		while ( slot== NULL )
            slot = frameBuffer_->popSlot();

        unsigned char *p_Out_Frame = new unsigned char[WIDTH * HEIGHT * 3 / 2];
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


bool
Player::refresh()
{
    if( getState() == Stoped)
        return false;
    boost::shared_ptr<FrameBuffer::Slot> slot;

    slot = frameBuffer_->popSlot();

    while ( slot== NULL )
    {
        slot = frameBuffer_->popSlot();
        usleep(1000);
        //return false;
    }

    unsigned char *p_In_Frame = slot->getDataPtr();
    int outlen, inlen;
    inlen = slot->getPayloadSize();

    /*
    for( int i = 0; i <20; i++ )
            printf("%2X ",p_In_Frame[i]);
    cout << endl;
    */

    decoder_->decode( p_In_Frame, inlen, yuv_frameBuf_, outlen );

    //        std::cout << std::endl << "SizeIn: " << slot->getPayloadSize()
    //                  << " SizeOut: " << outlen << std::endl;

    if ( outlen > 0 )
    {

#ifdef __SHOW_CONSOLE_
        cout << "[Play]  : "
             << slot->getNumber() << " "
             << slot->getPayloadSize() << " "<<endl;
#endif

        unsigned char* yuv[3] = {yuv_frameBuf_,yuv_frameBuf_+ WIDTH*HEIGHT, yuv_frameBuf_ +WIDTH*HEIGHT*5/4};

        YUV420p_to_RGB24(yuv,bmp_frameBuf_,WIDTH,HEIGHT);
        //fwrite ( bmp_frameBuf_, WIDTH*HEIGHT*3/*outlen+54*/, 1, pFile_ );

        //return bmp_frameBuf_;
        //pixmap->loadFromData(image_buf, 800*480*4+54, "bmp", NULL);
        return true;
    }
    //cout << endl << slot->getNumber() << " " << slot->getPayloadSize() << endl << endl;

    return false;
}
