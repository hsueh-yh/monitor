//
//  frame-data.cpp
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//


#include "common.h"
#include "frame-data.h"
#include "utils.h"

#define PREFIX_META_NCOMP 5

using namespace std;


//******************************************************************************
//  NetworkData
//******************************************************************************

NetworkData::NetworkData(unsigned int dataLength, const unsigned char* rawData)
{
    copyFromRaw(dataLength, rawData);
}

NetworkData::NetworkData(const NetworkData& networkData)
{
    copyFromRaw(networkData.getLength(), networkData.getData());
    isValid_ = networkData.isValid();
}

NetworkData::~NetworkData(){
    if (data_ && isDataCopied_)
        free(data_);
}

void NetworkData::copyFromRaw(unsigned int dataLength, const unsigned char* rawData)
{
    length_ = dataLength;
    data_ = (unsigned char*)malloc(dataLength);
    memcpy((void*)data_, (void*)rawData, length_);
    isDataCopied_ = true;
}

//******************************************************************************
//  PacketData
//******************************************************************************

const PacketData::PacketMetadata PacketData::ZeroMetadata = {0, 0, 0};
const PacketData::PacketMetadata PacketData::BadMetadata = {-1, -1, -1};


PacketData::PacketMetadata
PacketData::getMetadata()
{
    PacketMetadata meta;
    meta.packetRate_ = -1;
    meta.timestamp_ = -1;
    
    return meta;
}

int
PacketData::packetFromRaw(unsigned int length,
                                  unsigned char *data,
                                  PacketData **packetData)
{
    if (FrameData::isValidHeader(length, data))
    {
        FrameData *frameData = new FrameData();
        
        frameData->length_ = length;
        frameData->data_ = data;
        
        if (RESULT_GOOD(frameData->initFromRawData(frameData->length_,
                                                   frameData->data_)))
        {
            frameData->isValid_ = true;
            *packetData = frameData;
            return RESULT_OK;
        }
        
        delete frameData;
    }

    if (NdnAudioData::isValidHeader(length, data))
    {
        NdnAudioData *audioData = new NdnAudioData();

        audioData->length_ = length;
        audioData->data_ = data;
        
        if (RESULT_GOOD(audioData->initFromRawData(audioData->length_,
                                                   audioData->data_)))
        {
            audioData->isValid_ = true;
            *packetData = audioData;
            return RESULT_OK;
        }
        
        delete audioData;
    }

    
    return RESULT_ERR;
}

PacketData::PacketMetadata
PacketData::metadataFromRaw(unsigned int length,
                            const unsigned char *data)
{
    if (FrameData::isValidHeader(length, data))
        return FrameData::metadataFromRaw(length, data);
    
    if (NdnAudioData::isValidHeader(length, data))
        return NdnAudioData::metadataFromRaw(length, data);
    
    return BadMetadata;
}


//******************************************************************************
//  NdnFrameData
//******************************************************************************


FrameData::FrameData(unsigned int length, const unsigned char* rawData):
    PacketData(length, rawData)
{
    isValid_ = RESULT_GOOD(initFromRawData(length_, data_));
}
FrameData::FrameData(const EncodedImage &frame,
                           unsigned int segmentSize)
{
    initialize(frame, segmentSize);
}

FrameData::FrameData(const EncodedImage &frame, unsigned int segmentSize,
                           PacketMetadata &metadata)
{
    initialize(frame, segmentSize);
    ((FrameDataHeader*)(&data_[0]))->metadata_ = metadata;
}

FrameData::~FrameData()
{
}

//******************************************************************************

int
FrameData::getFrame(webrtc::EncodedImage &frame)
{
    if (!isValid())
        return RESULT_ERR;
    
    frame = frame_;
    
    return RESULT_OK;
}

void
copyFrame(const webrtc::EncodedImage &frameOriginal,
          webrtc::EncodedImage &frameCopy)
{
    
}

PacketData::PacketMetadata
FrameData::getMetadata()
{
    PacketMetadata meta = PacketData::getMetadata();
    
    if (isValid())
        meta = getHeader().metadata_;
    
    return meta;
}

void
FrameData::setMetadata(PacketMetadata &metadata)
{
    if (isValid())
        ((FrameDataHeader*)(&data_[0]))->metadata_ = metadata;
}

bool
FrameData::isValidHeader(unsigned int length, const unsigned char *data)
{
    unsigned int headerSize = sizeof(FrameDataHeader);
    
    if (length >= headerSize)
    {
        FrameDataHeader header = *((FrameDataHeader*)(&data[0]));

        if (header.headerMarker_ == NDNRTC_FRAMEHDR_MRKR &&
            header.bodyMarker_ == NDNRTC_FRAMEBODY_MRKR)
            return true;
    }
    
    return false;
}

PacketData::PacketMetadata
FrameData::metadataFromRaw(unsigned int length, const unsigned char *data)
{
    if (FrameData::isValidHeader(length, data))
    {
        FrameData::FrameDataHeader header = *((FrameDataHeader*)data);
        return header.metadata_;
    }
    
    return PacketData::BadMetadata;
}

//******************************************************************************

void
FrameData::initialize(const EncodedImage &frame, unsigned int segmentSize)
{
    unsigned int headerSize = sizeof(FrameDataHeader);
    unsigned int allocSize = (unsigned int)ceil((double)(frame._length+headerSize)/(double)segmentSize)*segmentSize;
    
    length_ = frame._length+headerSize;
    isDataCopied_ = true;
    data_ = (unsigned char*)malloc(allocSize);
    memset(data_, 0, allocSize);
    
    // copy frame data with offset of header
    memcpy(data_+headerSize, frame._buffer, frame._length);
    
    // setup header
    ((FrameDataHeader*)(&data_[0]))->encodedWidth_ = frame._encodedWidth;
    ((FrameDataHeader*)(&data_[0]))->encodedHeight_ = frame._encodedHeight;
    ((FrameDataHeader*)(&data_[0]))->timeStamp_ = frame._timeStamp;
    ((FrameDataHeader*)(&data_[0]))->capture_time_ms_ = frame.capture_time_ms_;
    ((FrameDataHeader*)(&data_[0]))->completeFrame_ = frame._completeFrame;
    
    isValid_ = RESULT_GOOD(initFromRawData(length_, data_));
}

int
FrameData::initFromRawData(unsigned int dataLength,
                              const unsigned char *rawData)
{
    unsigned int headerSize = sizeof(FrameDataHeader);
    FrameDataHeader header = *((FrameDataHeader*)(&rawData[0]));
    
    // check markers
    if ((header.headerMarker_ != NDNRTC_FRAMEHDR_MRKR ||
         header.bodyMarker_ != NDNRTC_FRAMEBODY_MRKR) ||
        dataLength < headerSize)
        return RESULT_ERR;
    
    int32_t size = webrtc::CalcBufferSize(webrtc::kI420, header.encodedWidth_,
                                          header.encodedHeight_);
        
    frame_ = EncodedImage(const_cast<uint8_t*>(&rawData[headerSize]),
                                  dataLength-headerSize, size);
    frame_._encodedWidth = header.encodedWidth_;
    frame_._encodedHeight = header.encodedHeight_;
    frame_._timeStamp = header.timeStamp_;
    frame_.capture_time_ms_ = header.capture_time_ms_;
    frame_._frameType = header.frameType_;
    frame_._completeFrame = header.completeFrame_;

    return RESULT_OK;
}

