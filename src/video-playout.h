#ifndef VIDEOPLAYOUT_H
#define VIDEOPLAYOUT_H

#include <QLabel>
#include "playout.h"
#include "video-consumer.h"

class VideoPlayout : public Playout
{
public:
    VideoPlayout(Consumer *consumer);
    ~VideoPlayout();

    int
    start(int playbackAdjustment = 0);

    //OnFrameSkipped onFrameSkipped_;

private:
    // this flags indicates whether frames should be played out
    // (unless new full key frame received)
    bool validGop_;
    PacketNumber currentKeyNo_;

    bool
    playbackPacket( int64_t packetTsLocal,
                    unsigned char *data,
                    unsigned int size,
                    PacketNumber playbackPacketNo,
                    PacketNumber sequencePacketNo,
                    PacketNumber pairedPacketNo,
                    bool isKey, double assembledLevel);

    VideoConsumer*
    getVideoConsumer()
    { return (VideoConsumer*)consumer_; }
};

#endif // VIDEOPLAYOUT_H
