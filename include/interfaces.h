#ifndef _INTERFACES_H_
#define _INTERFACES_H_

#include <string>

#include "params.h"


/**
  *This interface defines external renderers.
  *Each time, the frame is ready to be rendered, library calls
  *getFrameBuffer which should return a pointer to the buffer where library
  *can copy RGB frame data. Once this data is copied, library makes
  *renderRGBFrame call and passes the same buffer with additional parameters
  *so the renderer can perform rendering operations.
 */
class IExternalRenderer
{
public:
    /**
      *Should return allocated buffer big enough to store RGB frame data
      *(width*height*3) bytes.
      *@param width Width of the frame (NOTE: width can change during run)
      *@param height Height of the frame (NOTE: height can change during run)
      *@return Allocated buffer where library can copy RGB frame data
     */
    virtual uint8_t *getFrameBuffer(int width, int height) = 0;

    /**
      *This method is called every time new frame is available for rendering.
      *This method is called on the same thread as getFrameBuffer was called.
      *@param timestamp Frame's timestamp
      *@param width Frame's width (NOTE: width can change during run)
      *@param height Frame's height (NOTE: height can change during run)
      *@param buffer Buffer with the RGB frame data (the same that was
      *returned from getFrameBuffer call)
      *@see getFrameBuffer
     */
    virtual void renderYUVFrame(int64_t timestamp, int width, int height,
                                 const uint8_t *buffer) = 0;
};


/**
      *Interface for remote stream observer. Gets updates for notable events
      *occuring while remote stream is fetched.
     */
    typedef enum _ConsumerStatus {
        ConsumerStatusStopped,
        ConsumerStatusNoData,   // consumer has started but no data received yet
        ConsumerStatusAdjusting,  // consumer re-adjusts interests' window
        ConsumerStatusBuffering,    // consumer has finished chasing and is
                                    // buffering frames unless buffer reaches
                                    // target size
        ConsumerStatusFetching, // consumer has finished buffering and switched
        // to normal operating fetching mode
    } ConsumerStatus;


/**
  *playback events occur at the moment when frame is taken from the buffer
  *for playback by playback mechanism. this means, the frame taken has
  *reached it's deadline and should be played back or skipped due to
  *different reasons:
 */
typedef enum _PlaybackEvent
{
    PlaybackEventDeltaSkipIncomplete, // consumer had to skip frame as it was
    // not fully fetched (incomplete)
    PlaybackEventDeltaSkipInvalidGop, // consumer had to skip frame due to
    // receiving incomplete frame previously,
    // even if the current frame is complete
    PlaybackEventDeltaSkipNoKey,      // consumer had to skip frame as there
    // is no key frame for frame's GOP
    PlaybackEventKeySkipIncomplete    // consumer had to skip key frame as
    // it is incomplete
} PlaybackEvent;


class IConsumerObserver
{
public:
    /**
      *Called when consumer updates its' status
     */
    virtual void
    onStatusChanged(ConsumerStatus newStatus) = 0;

    /**
      *Called each time consumer encounters rebuffering event (no data
      *received for ~1200ms)
     */
    virtual void
    onRebufferingOccurred() = 0;

    /**
      *Called each time consumer encounters new buffer event
     */
    virtual void
    onPlaybackEventOccurred(PlaybackEvent event, unsigned int frameSeqNo) = 0;

    /**
      *Called when stream has switched to another thread
     */
    virtual void
    onThreadSwitched(const std::string &threadName) = 0;
};


/******************************************************************************
  *This is an interface for the NDN-RTC library
 ******************************************************************************/
class IMMNdnLibrary
{
public:
    /**
      *Sets library observer
      *@param observer Pointer to the observer object
      *@see INdnRtcLibraryObserver
     */
//    virtual void
//    setObserver(INdnRtcLibraryObserver *observer) = 0;

    /**
      *Adds new remote stream and starts fetching from its' first thread
      *@param remoteSessionPrefix Remote producer's session prefix returned
      *                           by previsous setRemoteSessionObserver call
      *@param threadName Thread name to fetch from
      *@param params Media stream parameters
      *@param generalParams General NDN-RTC parameters
      *@param consumerParams General consumer parameters
      *@param renderer Pointer to the object which conforms to the
      *                IExternalRenderer interface (video only)
      *@return Remote stream prefix on success and empty string on failure
      *@see setRemoteSessionObserver
     */
    virtual std::string
    addRemoteStream(std::string &remoteStreamPrefix,
                    const std::string &threadName,
                    const MediaStreamParams &params,
                    const GeneralParams &generalParams,
                    const GeneralConsumerParams &consumerParams,
                    IExternalRenderer *const renderer) = 0;

    /**
      *Removes remote stream and stops fetching from it
      *@param streamPrefix Stream prefix returned by previous addRemoteStream
      *                    call
      *@return Full log file path on success, empty string on failure
     */
    virtual std::string
    removeRemoteStream(const std::string &streamPrefix) = 0;

    /**
      *Sets remote stream obsever and starts sending fetching updates to it
      *@param streamPrefix Stream prefix returned by previous addRemoteStream
      *                    call
      *@param observer Pointer to an object which conforms to the
      *                IConsumerObserver interface
      *@return RESULT_OK on success, RESULT_ERR on failure
     */
    virtual int
    setStreamObserver(const std::string &streamPrefix,
                      IConsumerObserver *const observer) = 0;

    /**
      *Removes remote stream observer and stops sending updates to it
      *@param streamPrefix Stream prefix returned by previous addRemoteStream
      *                    call
      *@return RESULT_OK on success, RESULT_ERR on failure
      *@see setStreamObserver
     */
    virtual int
    removeStreamObserver(const std::string &streamPrefix) = 0;

    /**
      *Returns currently fetched media thread name
      *@param streamPrefix Stream prefix returned by previous addRemoteStream
      *                    call
      *@see addRemoteStream
     */
    virtual std::string
    getStreamThread(const std::string &streamPrefix) = 0;

    /**
      *Forces specified fetching stream to switch active media thread
      *@param streamPrefix Stream prefix returned by previous addRemoteStream
      *                    call
      *@param threadName Name of the thread which should be used for
      *                  fetching media. All threads are returned in SessionInfo
      *                  object to remote session observer
      *@return RESULT_OK on success, RESULT_ERR on failure
      *@see setRemoteSessionObserver
     */
    virtual int
    switchThread(const std::string &streamPrefix,
                 const std::string &threadName) = 0;

};


#endif
