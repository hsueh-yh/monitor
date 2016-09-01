#include "frame-data.h"

MediaData::MediaData(unsigned int length, const unsigned char *rawData)
{
    copyFromRaw( length, rawData );
}

MediaData::MediaData(const MediaData &mediaData)
{
    copyFromRaw( mediaData.getLength(), mediaData.getData() );
}

MediaData::~MediaData()
{
    if( length_ && isDataCopied_ )
        free(data_);
}

MediaData::copyFromRaw(unsigned int dataLength, const unsigned char *rawData)
{
    length_ = dataLength;
    data_ = (unsigned char*) malloc ( length_ );
    memcpy( data_,rawData,length_ );
    isDataCopied_ = true;
}
