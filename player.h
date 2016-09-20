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

//#define WIDTH 1080
//#define HEIGHT 720
#define WIDTH 640
#define HEIGHT 480


class Player
{
public:

    typedef enum{
        Stoped = -1,
        Ready = 0,
        Started = 1
    }State;

    Player();

	~Player();

    bool init (boost::shared_ptr<FrameBuffer> frameBuffer);

    void start();

    void stop();

	void writeFile ();

    bool refresh();


    void lock()  { syncMutex_.lock(); }
    void unlock() { syncMutex_.unlock(); }


    State getState()
    { lock(); Player::State stat = state_; unlock();return stat;  }


    unsigned char* imageProcess(const void* p,unsigned char* dst);

    unsigned char* bmp_frameBuf_;

    State state_;
    std::recursive_mutex syncMutex_;

protected:

    boost::shared_ptr<FrameBuffer> frameBuffer_;

    FILE *pFile_;
	Decoder *decoder_;

    unsigned char * yuv_frameBuf_/**, bmp_frameBuf_*/;


    bool processing();

    void changetoState(Player::State stat);

};


#endif /* PLAYER_H_ */
