/*
 * player.h
 *
 *  Created on: Jun 16, 2016
 *      Author: xyh
 */


#ifndef __PLAYER_H_
#define __PLAYER_H_

#include <iostream>
#include <boost/shared_ptr.hpp>

#include <QLabel>
#include <QPixmap>

#include "decoder.h"
#include "frame-buffer.h"
#include "consumer.h"


//static unsigned char bmp_h[] =
//{
//                0x42,0x4d,            //BM
//                0x42,0x70,0x17,0x00,  // 172000+66
//                0x00,0x00,0x00,0x00,
//                0x36,0x00,0x00,0x00,  //bmp_data offset
//                0x28,0x00,0x00,0x00,
//                0x20,0x03,0x00,0x00,   //width
//                0x20,0xfe,0xff,0xff,   //hieght
//                0x01,0x00,
//                0x20,0x00,             //32bit
//                0x00,0x00,0x00,0x00,
//                0x00,0x70,0x17,0x00,  //bmp_data size
//                0x00,0x00,0x00,0x00,
//                0x00,0x00,0x00,0x00,
//                0x00,0x00,0x00,0x00,
//                0x00,0x00,0x00,0x00
//};


class Player
{
public:
    Player();

	~Player();

    bool init (boost::shared_ptr<FrameBuffer> frameBuffer);

	void writeFile ();

    unsigned char* getPixmap();


    unsigned char* imageProcess(const void* p,unsigned char* dst);



    QLabel *label;
    QPixmap *pixmap;

    unsigned char* bmp_frameBuf_;

private:
    boost::shared_ptr<FrameBuffer> frameBuffer_;

	FILE *pFile_, *pFile1_;
	Decoder *decoder_;

    unsigned char * yuv_frameBuf_/**, bmp_frameBuf_*/;

    int status_;
};


#endif /* PLAYER_H_ */
