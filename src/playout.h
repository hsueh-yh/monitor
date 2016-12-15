//
//  playout.h
//  mtndn
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//  Created: 3/19/14
//

#ifndef _playout_
#define _playout_

#include "mtndn-object.h"
#include "frame-buffer.h"
#include "jitter-timing.h"

class Consumer;


/**
  *Base class for playout mechanisms. The core playout logic is similar
  *for audio and video streams. Differences must be implemented in
  *overriden method playbackPacket which is called from main playout
  *routine each time playout timer fires. Necessary information is
  *provided as arguments to the method.
 */
class Playout : public MtNdnComponent
{
public:
    static const int BufferCheckInterval;

    Playout(Consumer *consumer);
    virtual ~Playout();

    virtual int
    init(void *frameConsumer);

    virtual int
    start(int initialAdjustment = 0);

    virtual int
    stop();

    bool
    isRunning()
    { return isRunning_; }

    void
    setPlaybackAdjustment(int playbackAdjustment)
    { playbackAdjustment_ = playbackAdjustment; }

protected:

    bool isRunning_;

    bool isInferredPlayback_;
    int64_t lastPacketTs_;
    unsigned int inferredDelay_;
    int playbackAdjustment_;
    int64_t bufferCheckTs_;

    Consumer *consumer_;
    ptr_lib::shared_ptr<FrameBuffer> frameBuffer_;

    ptr_lib::shared_ptr<JitterTiming> jitterTiming_;
    ptr_lib::mutex playoutMutex_;

    void *encodedFrameConsumer_;
    //unsigned char *data_;
    vector<uint8_t> *vdata_;

    /**
      *This method should be overriden by derived classes for
      *media-specific playback (audio/video)
      *@param packetTsLocal Packet local timestamp
      *@param data Packet data retrieved from the buffer
      *@param packetNo Packet playback number provided by a producer
      *@param isKey Indicates, whether the packet is a key frame (@note
      *always false for audio packets)
      *@param assembledLevel Ratio indicating assembled level of the
      *packet: number of fetched segments divided by total number of
      *segments for this packet
     */
    virtual bool
    playbackPacket( int64_t packetTsLocal,
                    unsigned char *data,
                    unsigned int size,
                    PacketNumber playbackPacketNo,
                    PacketNumber sequencePacketNo,
                    PacketNumber pairedPacketNo,
                    bool isKey, double assembledLevel) = 0;

    bool
    processPlayout();

    // update playout time if previous was inffered
    void
    updatePlaybackAdjustment(int64_t ts);

    int
    playbackDelayAdjustment(int playbackDelay);

/*
    int
    avSyncAdjustment(int64_t nowTimestamp, int playbackDelay);
*/
    void
    checkBuffer();

};

#endif /*defined(__mtndn__playout__) */
