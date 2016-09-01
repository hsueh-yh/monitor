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


class MediaData {

public:
    MediaData() : length_(0), data_(NULL) {}
    MediaData(unsigned int length, const unsigned char* rawData );
    MediaData( const MediaData& mediaData );

    virtual ~MediaData();

    int getLength() const { return length_; }
    unsigned char* getData() const { return data_; }

protected:

    unsigned int length_;
    unsigned char* data_ = NULL;
    bool isDataCopied_ = false;

    virtual int
    initFromRawData(unsigned int dataLength,
                    const unsigned char* rawData) = 0;

    void copyFromRaw(unsigned int dataLength,
                     const unsigned char* rawData);
};


class SegmentData : public MediaData {

public:
    typedef struct SegmentMetaInfo {
        uint32_t interestNonce;
        int64_t interestArrivalTimestamp;
        int64_t generationDelay;
    } __attribute__((packed)) SegmentMetaInfo ;

    typedef SegmentMetaInfo SegmentHeader;

    SegmentData(){}
    SegmentData( const unsigned char *segmentData, const unsigned int dataSize,
                 SegmentMetaInfo metadata = (SegmentMetaInfo){0,0,0} );

    SegmentMetaInfo getMetaData() { return ((SegmentMetaInfo*)(&data_[0])); }

    unsigned char* getSegmentData() { return data_ + getHeaderSize(); }

    unsigned int getHeaderSize() { return sizeof(SegmentMetaInfo); }

    unsigned int getSegmentDataLenth() { return length_ - getHeaderSize(); }

    int
    initFromRawData(unsigned int dataLength,
                        const unsigned char* rawData);

private:
    SegmentMetaInfo metaInfo_;
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

//SegmentData::SegmentMetaInfo
struct SegmentHeader {
    uint32_t interestNonce;
    int64_t interestArrivalTimestamp;
    int64_t generationDelay;
} __attribute__((packed)) ;

typedef struct _SegmentMetaInfo {
    int32_t interestNonce_;
    int64_t interestArrivalMs_;
    int32_t generationDelayMs_;
} __attribute__((packed)) SegmentMetaInfo;

typedef SegmentHeader SegmentMetaInfo;

struct SegmentData {
    SegmentHeader header_;
    unsigned char* buf_;
};

#endif /* defined(__frame_data__) */
