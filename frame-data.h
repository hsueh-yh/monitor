//
//  frame-data.h
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

#ifndef __frame_data__
#define __frame_data__

#include <boost/crc.hpp>
#include <ndn-cpp/name.hpp>

#include "common.h"


enum FrameType{
    IFrame = 0,
    PFrame = 1,
    BFrame = 2
};

struct FrameDataHeader {
        uint32_t					frameNumber;
        uint32_t                    encodedWidth_;
        uint32_t                    encodedHeight_;
        uint32_t					length_;			// data length (without header)
        uint32_t                    timeStamp_;
        FrameType					frameType_;
        bool                        completeFrame_;
} __attribute__((packed));

struct FrameData {
    FrameDataHeader header_;
    unsigned char* 	buf_;
};


#endif /* defined(__frame_data__) */
