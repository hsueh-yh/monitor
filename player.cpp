/*
 * player.cpp
 *
 *  Created on: Jun 17, 2016
 *      Author: xyh
 */

#include "player.h"
#include "logger.hpp"

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


static bool
YV12ToBGR24_Native(unsigned char* pYUV,unsigned char* pBGR24,int width,int height)
{
    if (width < 1 || height < 1 || pYUV == NULL || pBGR24 == NULL)
        return false;
    const long len = width * height;
    unsigned char* yData = pYUV;
    unsigned char* vData = &yData[len];
    unsigned char* uData = &vData[len >> 2];

    int bgr[3];
    int yIdx,uIdx,vIdx,idx;
    for (int i = 0;i < height;i++){
        for (int j = 0;j < width;j++){
            yIdx = i * width + j;
            vIdx = (i/2) * (width/2) + (j/2);
            uIdx = vIdx;

            bgr[0] = (int)(yData[yIdx] + 1.732446 * (uData[vIdx] - 128));                                    // b分量
            bgr[1] = (int)(yData[yIdx] - 0.698001 * (uData[uIdx] - 128) - 0.703125 * (vData[vIdx] - 128));    // g分量
            bgr[2] = (int)(yData[yIdx] + 1.370705 * (vData[uIdx] - 128));                                    // r分量

            for (int k = 0;k < 3;k++){
                idx = (i * width + j) * 3 + k;
                if(bgr[k] >= 0 && bgr[k] <= 255)
                    pBGR24[idx] = bgr[k];
                else
                    pBGR24[idx] = (bgr[k] < 0)?0:255;
            }
        }
    }
    return true;
}


Player::Player():
    decoder_(new Decoder()),
    p_In_Frame(new unsigned char[WIDTH * HEIGHT * 3 / 2]),
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
    LOG(INFO) << "[Player] dtor" << endl;
    LOG(WARNING) << "[Player] dtor" << endl;
}


bool
Player::init (ptr_lib::shared_ptr<FrameBuffer> frameBuffer)
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

    LOG(INFO) << "[Player] Stoping" << endl;

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
        ptr_lib::shared_ptr<FrameBuffer::Slot> slot;
        //cout << frameBuffer_->status_ << endl;
        //while(frameBuffer_->status_ != STARTED);

        while ( slot== NULL )
            slot = frameBuffer_->acquireData();

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


unsigned int
Player::refresh()
{
    if( getState() == Stoped)
        return 0;
    ptr_lib::shared_ptr<FrameBuffer::Slot> slot;

    slot = frameBuffer_->acquireData();

    while ( slot== NULL )
    {
       // LOG(INFO) << "[Player] wait frame " << endl;
        slot = frameBuffer_->acquireData();
        usleep(10000);
        //return false;
    }

    LOG(INFO) << "[Player] get " << slot->getFrameNo() << endl;

    unsigned char *p_In_Frame = slot->getDataPtr();
    int outlen, inlen;
    inlen = slot->getPayloadSize();

    decoder_->decode( p_In_Frame, inlen, yuv_frameBuf_, outlen );

    if ( outlen > 0 )
    {
       // LOG(INFO) << "[Player] play " << slot->getNumber() << endl;

        YV12ToBGR24_Native(yuv_frameBuf_,bmp_frameBuf_,WIDTH,HEIGHT);
    }

    return frameBuffer_->getBufSize();
}


unsigned int
Player::refresh( int a)
{
    if( getState() == Stoped)
        return 0;

    unsigned int bufsize;
    bufsize = frameBuffer_->acquireData( p_In_Frame );

    while ( bufsize == NULL )
    {
       // LOG(INFO) << "[Player] wait frame " << endl;
        bufsize = frameBuffer_->acquireData( p_In_Frame );
        usleep(10*1000);
        //return false;
    }

    LOG(INFO) << "[Player] get NALU ( size = " << bufsize << " )" << endl;

    int outlen;
    decoder_->decode( p_In_Frame, bufsize, yuv_frameBuf_, outlen );

    if ( outlen > 0 )
    {
       // LOG(INFO) << "[Player] play " << slot->getNumber() << endl;
        YV12ToBGR24_Native(yuv_frameBuf_,bmp_frameBuf_,WIDTH,HEIGHT);
    }

    return frameBuffer_->getBufSize();
}
