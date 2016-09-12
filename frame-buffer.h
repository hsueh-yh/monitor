/*
 * frame-buffer.h
 *
 *  Created on: Jun 15, 2016
 *      Author: xyh
 */

#ifndef FRAME_BUFFER_H_
#define FRAME_BUFFER_H_

#include <iostream>
#include <functional>
#include <boost/thread/mutex.hpp>
#include <thread>
#include <mutex>

#include <queue>
#include <vector>
#include <map>

#include <ndn-cpp/name.hpp>
#include <ndn-cpp/data.hpp>

#include "frame-data.h"

#define SLOTNUM 50

using namespace std;
using namespace ndn;


class IFrameBufferCallback
{
public:
    virtual void
    onSegmentNeeded( const FrameNumber frameNo, const SegmentNumber segNo ) = 0;
};


class FrameBuffer
{
public:

    class Slot
    {
    public:

        class Segment
        {
        public:
            enum State {
                StateFetched = 1<<0,   // segment has been fetched already
                StatePending = 1<<1,   // segment awaits it's interest to
                                        // be answered
                StateMissing = 1<<2,   // segment was timed out or
                                        // interests has not been issued yet
                StateNotUsed = 1<<3    // segment is no used in frame
                                        // assembling
            };

            Segment();
            ~Segment();

            //upward communication with Slot
            //************************************************

            /**
             * @brief Moves segment into Pending state and updaets following
             * @param:
             * - requestTimeUsec
             * - interestName
             * - interestNonce
             * - reqCounter
             */
            void interestIssued(const uint32_t& nonceValue);

            /**
             * @brief   Moves segment into Fetched state and updates following
             * @param:
             * - dataName
             * - dataNonce
             * - generationDelay
             * - arrivalTimeUsec
             */
            void
            dataArrived(const SegmentData::SegmentMetaInfo& segmentMeta);

            /**
             * @brief Moves segment into Missing state if it was in Pending
             *          state before
             */
            void markMissed();

            /**
             * @brief Discards segment by swithcing it to NotUsed state and
             *          reseting all the attributes
             */
            void discard();

            /**
             * @brief Returns true if the interest issued for a segment was
             *          answered by a producer
             */
            bool
            isOriginal();


            //setter and getter
            //************************************************
            State
            getState() const { return state_; }

            void
            setState( State state ){ state_ = state; }

            void
            setNumber(SegmentNumber number) { segmentNumber_ = number; }

            SegmentNumber
            getSegmentNumber() const { return segmentNumber_; }

            int64_t
            getRequestTimeUsec()
            { return requestTimeUsec_; }

            int64_t
            getArrivalTimeUsec()
            { return arrivalTimeUsec_; }

            int64_t
            getRoundTripDelayUsec()
            {
                if (arrivalTimeUsec_ <= 0 || requestTimeUsec_ <= 0)
                    return -1;
                return (arrivalTimeUsec_-requestTimeUsec_);
            }

            void
            setPayloadSize(unsigned int payloadSize)
            { payloadSize_ = payloadSize; }

            unsigned int
            getPayloadSize() const { return payloadSize_; }

            void
            setDataPtr(const unsigned char* dataPtr)
            { dataPtr_ = const_cast<unsigned char*>(dataPtr); }

            unsigned char*
            getDataPtr() const { return dataPtr_; }

            void
            setIsParity(bool isParity)
            { isParity_ = isParity; }

            bool
            isParity()
            { return isParity_; }

            SegmentData::SegmentMetaInfo
            getMetadata() const;

            static std::string
            stateToString(State s)
            {
                switch (s)
                {
                    case StateFetched: return "Fetched"; break;
                    case StateMissing: return "Missing"; break;
                    case StatePending: return "Pending"; break;
                    case StateNotUsed: return "Not used"; break;
                    default: return "Unknown"; break;
                }
            }


        protected:

            State state_;

            SegmentNumber segmentNumber_;

            uint32_t dataNonce_; // nonce value provided with the
                                 // segment's meta data
            uint32_t interestNonce_; // nonce used with issuing interest
                                     // for this segment. if dataNonce_
                                     // and interestNonce_ coincides,
                                     // this means that the interest was
                                     // answered by a producer

            int64_t requestTimeUsec_, // local timestamp when the interest
                                      // for this segment was issued
            arrivalTimeUsec_, // local timestamp when data for this
                              // segment has arrived
            consumeTimeMs_;   // remote timestamp (milliseconds)
                              // when the interest, issued for
                              // this segment, has been consumed
                              // by a producer. could be 0 if
                              // interest was not answered by
                              // producer directly (cached on
                              // the network)
            int32_t generationDelayMs_;  // in case if segment arrived
                                         // straight from producer, it
                                         // puts a delay between receiving
                                         // an interest and answering it
                                         // into the segment's meta data
                                         // header, otherwise - 0

            int requestCounter_; // indicates, how many times segment was
                             // requested

            unsigned char* dataPtr_;    // pointer to the payload data
            unsigned int payloadSize_;  // size of actual data payload
                                        // (without segment header)

            bool isParity_;


            //************************************************
            void reset();

        }; //class Segment

        class Comparator
		{
		public:
			Comparator(bool inverted = false):inverted_(inverted){}

            bool operator() (const boost::shared_ptr<Slot> slot1, const boost::shared_ptr<Slot> slot2)
			{
                return slot1->getFrameNumber() > slot2->getFrameNumber();
			}
/*
            bool operator < (Slot slot1, Slot slot2)
			{
                return slot1.slotNumber_ < slot2.slotNumber_;
			}
*/

        private:
			bool inverted_;
		};

        enum State {
            StateFree = 1<<0,  // slot is free for being used
            StateNew = 1<<1,   // slot is being used for assembling, but has
                            // not recevied any data segments yet
            StateAssembling = 1<<2,    // slot is being used for assembling and
                                    // already has some data segments arrived
            StateReady = 1<<3, // slot assembled all the data and is ready for
                            // decoding a frame
            StateLocked = 1<<4 // slot is locked for decoding
        }; // enum State

        Slot();
        ~Slot();


        //upward communication with FrameBuffer
        //************************************************
        /**
         * Moves frame into Pending state and updaets following
         * attributes:
         * - requestTimeUsec
         * - interestName
         * - reqCounter
         */
        void addInterest(Interest &interest);

        /**
         * @brief append data to the slot
         * @param ndn data
         */
        void
        appendData( const ndn::Data &data );

        /**
         * @brief Moves frame into Missing state if it was in Pending
         *          state before
         */
        void markMissed(const Interest &interest);

        /**
         * @brief Discards frame by swithcing it to NotUsed state and
         *          reseting all the attributes
         */
        void discard();

        void
        getMissedSegments(std::vector<SegmentNumber>& missedSegments);


        //getter and setter
        //************************************************
        State
        getState() const { return state_; }

        void
        setState( State state ) { state_ = state; }

        const Name&
        getPrefix() { return prefix_; }

        void
        setPrefix(const Name& prefix)
        { prefix_ = prefix; }

        FrameNumber
        getFrameNumber() const { return frameNumber_; }

        void
        setFrameNumber(FrameNumber number) { frameNumber_ = number; }

        int64_t
        getRequestTimeUsec()
        { return requestTimeUsec_; }

        int64_t
        getArrivalTimeUsec()
        { return readyTimeUsec_; }

        int64_t
        getRoundTripDelayUsec()
        {
            if (readyTimeUsec_ <= 0 || requestTimeUsec_ <= 0)
                return -1;
            return (readyTimeUsec_-requestTimeUsec_);
        }

        unsigned char*
        getDataPtr() const { return slotData_; }

        void
        setDataPtr(const unsigned char* dataPtr)
        { slotData_ = const_cast<unsigned char*>(dataPtr); }

        unsigned int
        getPayloadSize() const { return payloadSize_; }

        void
        setPayloadSize(unsigned int payloadSize)
        { payloadSize_ = payloadSize; }


        void
        lock()  { syncMutex_.lock(); }

        void
        unlock() { syncMutex_.unlock(); }


    protected:

        State           state_;

        Name            prefix_;
        FrameNumber     frameNumber_;

        int64_t         requestTimeUsec_, // local timestamp when the interest
                                  // for this frame was issued
                        //firstSegmentTimeUsec_, // local timestamp when first segment
                                  // for this frame has arrived
                        readyTimeUsec_; // local timestamp when this frame is ready

        int             nSegmentMissed,
                        nSegmentTotal,
                        nSegmentPending,
                        nSegmentReady;

        std::vector<boost::shared_ptr<Segment>>
                        freeSegments_;
        std::map<SegmentNumber, boost::shared_ptr<Segment>>
                        activeSegments_;

        unsigned char*  slotData_;    // pointer to the payload data
        unsigned int    payloadSize_;  // size of actual data payload
                                    // (without frame header)
        unsigned int    allocatedSize_ = 0,
                        assembledSize_ = 0;

        std::recursive_mutex syncMutex_;


        //downward communication with Segment
        //**************************************************
        void reset();

        boost::shared_ptr<Segment>
            pickFreeSegment();

        boost::shared_ptr<Segment>
            prepareSegment(SegmentNumber segNo);

        boost::shared_ptr<Segment>
            getSegment(SegmentNumber segNo);

        /**
         * @brief add segment data to slotData_
         *          (allocatedSize_ may be changed)
         * @param:
         * -segmentData
         * -segNo
         */
        unsigned char *addData( SegmentData segmentData, SegmentNumber segNo );

        void updateSlot();

        void freeActiveSegment(SegmentNumber segNo );
        void freeActiveSegment
                (std::map<SegmentNumber, boost::shared_ptr<Segment>>::iterator iter );

    };// class Slot


    enum State {
        Invalid = 0,
        Valid = 1
    };


    FrameBuffer():
        state_(Valid)
    {
        reset();
    }


    ~FrameBuffer()
    {
#ifdef __SHOW_CONSOLE_
        cout << "[FrameBuffer] dtor" << endl;
#endif
    }

    int init();

    void reset();

    void initialize( int slotNum=SLOTNUM );

    void stop() { setState( Invalid ); }

    //upward communication with Pipeliner
    //************************************************
    void interestIssued( Interest interest );

    void recvData(const ndn::ptr_lib::shared_ptr<Data>& data);

    void interestTimeout(const ndn::Interest &interest);

    void checkMissed();

    void acquireSlot ( MediaData *mediaData, FrameNumber frameNo );

    void
    registerCallback(IFrameBufferCallback* callback)
    { callback_ = callback; }


protected:

    /*
    typedef std::vector<FrameBuffer::Slot*> PlaybackQueueBase;

    class PlaybackQueue : public PlaybackQueueBase
    {
    public:
        PlaybackQueue(double playbackRate = 30.);

        int64_t
        getPlaybackDuration(bool estimate = true);

        void
        updatePlaybackDeadlines();

        void
        pushSlot(FrameBuffer::Slot* const slot);

        FrameBuffer::Slot*
        peekSlot();

        void
        popSlot();

        void
        updatePlaybackRate(double playbackRate);

        double
        getPlaybackRate() const { return playbackRate_; }

        void
        clear();

        int64_t
        getInferredFrameDuration()
        { //return lastFrameDuration_;
            return ceil(1000./playbackRate_);
        }

        void
        dumpQueue();

        std::string
        dumpShort();

    private:
        double playbackRate_;
        int64_t lastFrameDuration_;
        FrameBuffer::Slot::PlaybackComparator comparator_;

        void
        sort();
    };
*/


    State state_;

    PacketNumber playbackNo_;

    boost::shared_ptr<Slot> playbackSlot_;


    typedef boost::shared_ptr<Slot> SlotPtr;
    typedef
        priority_queue< SlotPtr, vector<SlotPtr>, Slot::Comparator>
    PlaybackQueue;

    PlaybackQueue playbackQueue_;
    std::vector<boost::shared_ptr<Slot> > freeSlots_;
    std::map<Name, boost::shared_ptr<Slot>> activeSlots_;


    IFrameBufferCallback* callback_;

    mutable std::recursive_mutex syncMutex_;

    //downward communication with Slot
    //************************************************
    boost::shared_ptr<FrameBuffer::Slot> getSlot(const ndn::Name& prefix, bool remove);

    void setSlot(const ndn::ptr_lib::shared_ptr<Data>& data, boost::shared_ptr<Slot> slot);

    void lock()  { syncMutex_.lock(); }

    void unlock() { syncMutex_.unlock(); }

    int getActiveSlotCount() { return activeSlots_.size(); }

};



#endif /* FRAME_BUFFER_H_ */
