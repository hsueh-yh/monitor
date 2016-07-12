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

#include "decoder.h"
#include "frame-buffer.h"

#define WIDTH 1080
#define HEIGHT 720


class Player
{
public:
    Player();

	~Player();

    bool init (boost::shared_ptr<FrameBuffer> frameBuffer);

	void writeFile ();

    bool refresh();

    unsigned char* imageProcess(const void* p,unsigned char* dst);

    unsigned char* bmp_frameBuf_;


private:
    boost::shared_ptr<FrameBuffer> frameBuffer_;

	FILE *pFile_, *pFile1_;
	Decoder *decoder_;

    unsigned char * yuv_frameBuf_/**, bmp_frameBuf_*/;

};


#endif /* PLAYER_H_ */
