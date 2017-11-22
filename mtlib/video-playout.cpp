#include "video-playout.h"

VideoPlayout::VideoPlayout(Consumer *consumer) :
    Playout(consumer)
{
    setDescription("VideoPlayout");
}

VideoPlayout::~VideoPlayout()
{}

//***********************************************************
int
VideoPlayout::start(int playbackAdjustment)
{
    validGop_ = false;
    currentKeyNo_ = 0;

    int res = Playout::start(playbackAdjustment);

    return res;
}

bool
VideoPlayout::playbackPacket(int64_t packetTsLocal,
                              vector<uint8_t> &data,
                              PacketNumber playbackPacketNo,
                              PacketNumber sequencePacketNo,
                              PacketNumber pairedPacketNo,
                              bool isKey, double assembledLevel)
{
    // render frame if we have one
    if (encodedFrameConsumer_)
    {
        /*
        bool pushFrameFurther = false;

        if (consumer_->getGeneralParameters().skipIncomplete_)
        {
            if (isKey)
            {
                if (assembledLevel >= 1)
                {
                    pushFrameFurther = true;
                    currentKeyNo_ = sequencePacketNo;
                    validGop_ = true;

                    LogTraceC << "new GOP with key: "
                    << currentKeyNo_ << endl;
                }
                else
                {
                    validGop_ = false;
                    (*statStorage_)[Indicator::SkippedIncompleteNum]++;
                    (*statStorage_)[Indicator::SkippedIncompleteKeyNum]++;

                    getVideoConsumer()->playbackEventOccurred(PlaybackEventKeySkipIncomplete,
                                                              sequencePacketNo);

                    LogWarnC << "key incomplete."
                    << " lvl " << assembledLevel
                    << " seq " << sequencePacketNo
                    << " abs " << playbackPacketNo
                    << endl;
                }
            }
            else
            {
                // update stat
                if (assembledLevel < 1.)
                {
                    (*statStorage_)[Indicator::SkippedIncompleteNum]++;

                    getVideoConsumer()->playbackEventOccurred(PlaybackEventDeltaSkipIncomplete,
                                                              sequencePacketNo);

                    LogWarnC << "delta incomplete."
                    << " lvl " << assembledLevel
                    << " seq " << sequencePacketNo
                    << " abs " << playbackPacketNo
                    << endl;
                }
                else
                {
                    if (pairedPacketNo != currentKeyNo_)
                    {
                        (*statStorage_)[Indicator::SkippedNoKeyNum]++;

                        getVideoConsumer()->playbackEventOccurred(PlaybackEventDeltaSkipNoKey, sequencePacketNo);

                        LogWarnC << "delta gop mismatch."
                        << " current " << currentKeyNo_
                        << " received " << pairedPacketNo
                        << " seq " << sequencePacketNo
                        << " abs " << playbackPacketNo
                        << endl;
                    }
                    else
                    {
                        if (!validGop_)
                        {
                            (*statStorage_)[Indicator::SkippedBadGopNum]++;

                            getVideoConsumer()->playbackEventOccurred(PlaybackEventDeltaSkipInvalidGop, sequencePacketNo);

                            LogWarnC << "invalid gop."
                            << " seq " << sequencePacketNo
                            << " abs " << playbackPacketNo
                            << " key " << currentKeyNo_
                            << endl;
                        }
                    }
                }

                validGop_ &= (assembledLevel >= 1);
                pushFrameFurther = validGop_ && (pairedPacketNo == currentKeyNo_);
            }
        }
        else
            pushFrameFurther  = true;

        // check for valid data
        pushFrameFurther &= (data != NULL);

#if RECORD
        if (pushFrameFurther)
        {
            PacketData::PacketMetadata meta = data_->getMetadata();
            frameWriter.writeFrame(frame, meta);
        }
#endif

        if (!pushFrameFurther)
        {
            if (onFrameSkipped_)
                onFrameSkipped_(playbackPacketNo, sequencePacketNo,
                                pairedPacketNo, isKey, assembledLevel);

            LogDebugC << "bad frame."
            << " type " << (isKey?"K":"D")
            << " lvl " << assembledLevel
            << " seq " << sequencePacketNo
            << " abs " << playbackPacketNo
            << endl;
        }
        else
        {
            LogDebugC << "playback."
            << " type " << (isKey?"K":"D")
            << " seq " << sequencePacketNo
            << " abs " << playbackPacketNo
            << " total " << (*statStorage_)[Indicator::PlayedNum]
            << endl;

            webrtc::EncodedImage frame;

            if (data)
                ((NdnFrameData*)data)->getFrame(frame);

            // update stat
            (*statStorage_)[Indicator::PlayedNum]++;
            if (isKey)
                (*statStorage_)[Indicator::PlayedKeyNum]++;

            ((IEncodedFrameConsumer*)frameConsumer_)->onEncodedFrameDelivered(frame, MtNdnUtils::unixTimestamp(), pushFrameFurther);
        }
*/

        LogTraceC
        //VLOG(LOG_INFO)
        << "PlayBackPacket "
        << consumer_->getPrefix()
        << "/" << playbackPacketNo
        << " size " << data.size()
        << " data " << (void*)data.data()
        << " pktTsLoc " << packetTsLocal
        //<< " seqPktNo " << sequencePacketNo
        //<< " pPktNo " << pairedPacketNo
        //<< " isKey " << isKey << " "
        //<< " asbLevel " << assembledLevel
        << std::endl;

        if( !data.empty() )
        {
            AVPacket frame;
            //av_init_packet(&frame);
            frame.data = data.data();
            frame.size = data.size();
            ((IEncodedFrameConsumer*)encodedFrameConsumer_)->onEncodedFrameDelivered(
                        frame,
                        MtNdnUtils::unixTimestamp());
        }

    }// if frameConsumer_

    return RESULT_OK;
}
