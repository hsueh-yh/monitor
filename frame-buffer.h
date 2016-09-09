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
    virtual void onSegmentNeeded( FrameNumber frameNo, SegmentNumber segNo );
};


class FrameBuffer
{
public:

    enum State {
        Invalid = 0,
        Valid = 1
    };

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

            /**
             * @brief Discards segment by swithcing it to NotUsed state and
             *          reseting all the attributes
             */
            void discard();

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
             * @brief Moves segment into Missing state if it was in Pending
             *          state before
             */
            void markMissed();

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
             * @brief Returns true if the interest issued for a segment was
             *          answered by a producer
             */
            bool
            isOriginal();

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
            setNumber(SegmentNumber number) { segmentNumber_ = number; }

            SegmentNumber
            getNumber() const { return segmentNumber_; }

            SegmentData::SegmentMetaInfo getMetadata() const;

            State
            getState() const { return state_; }

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
            setPrefix(const Name& prefix)
            { prefix_ = prefix; }

            const Name&
            getPrefix() { return prefix_; }

            void
            setIsParity(bool isParity)
            { isParity_ = isParity; }

            bool
            isParity()
            { return isParity_; }

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

            SegmentNumber segmentNumber_;
            Name prefix_;               //segment prefix:<.../FrameNo/SegmentNo>
            unsigned int payloadSize_;  // size of actual data payload
                                        // (without segment header)
            unsigned char* dataPtr_;    // pointer to the payload data
            State state_;

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
            int reqCounter_; // indicates, how many times segment was
                             // requested
            uint32_t dataNonce_; // nonce value provided with the
                                 // segment's meta data
            uint32_t interestNonce_; // nonce used with issuing interest
                                     // for this segment. if dataNonce_
                                     // and interestNonce_ coincides,
                                     // this means that the interest was
                                     // answered by a producer
            int32_t generationDelayMs_;  // in case if segment arrived
                                         // straight from producer, it
                                         // puts a delay between receiving
                                         // an interest and answering it
                                         // into the segment's meta data
                                         // header, otherwise - 0
            bool isParity_;

            void reset();

        }; //class Segment


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

        Slot();
        ~Slot();

        /**
         * @brief Discards frame by swithcing it to NotUsed state and
         *          reseting all the attributes
         */
        void discard();

        /**
         * Moves frame into Pending state and updaets following
         * attributes:
         * - requestTimeUsec
         * - interestName
         * - reqCounter
         */
        void addInterest(Interest &interest);

        /**
         * @brief Moves frame into Missing state if it was in Pending
         *          state before
         */
        void markMissed(const Interest &interest);

        /**
         * @brief append data to the slot
         * @param ndn data
         */
        void
        appendData( const ndn::Data &data );

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
        setNumber(FrameNumber number) { frameNumber_ = number; }

        FrameNumber
        getFrameNumber() const { return frameNumber_; }

        State
        getState() const { return state_; }

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
        setPrefix(const Name& prefix)
        { prefix_ = prefix; }

        const Name&
        getPrefix() { return prefix_; }

        void
        lock()  { syncMutex_.lock(); }

        void
        unlock() { syncMutex_.unlock(); }

    protected:

        State           state_;
        Name            prefix_;
        FrameNumber     frameNumber_;

        unsigned int    payloadSize_;  // size of actual data payload
                                    // (without frame header)
        unsigned char*  slotData_;    // pointer to the payload data
        unsigned int    allocatedSize_ = 0,
                        assembledSize_ = 0;

        int64_t         requestTimeUsec_, // local timestamp when the interest
                                  // for this frame was issued
                        firstSegmentTimeUsec_, // local timestamp when first segment
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

        std::recursive_mutex syncMutex_;


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


	FrameBuffer():
        count_(0),
        activeSlots_count_(0),
        state_(Started)
	{}


    ~FrameBuffer()
    {
#ifdef __SHOW_CONSOLE_
        cout << "[FrameBuffer] dtor" << endl;
#endif
    }

    int init();

    int reset();

    void initialize( int slotNum=SLOTNUM );

    void stop()
    {
        state_ = Stoped;
    }

    void
    lock()  { syncMutex_.lock(); }

    void
    unlock() { syncMutex_.unlock(); }

    void interestIssued( Interest interest );

    bool recvData(boost::shared_ptr<Slot> slot);

    void interestTimeout(const ndn::Interest &interest);

    void checkMissed(FrameNumber frameNo, SegmentNumber segNo);

    boost::shared_ptr<Slot> getSlot(const Name& prefix, bool remove);

    void setSlot(const ndn::ptr_lib::shared_ptr<Data>& data, boost::shared_ptr<Slot> slot);

    void recvData(const ndn::ptr_lib::shared_ptr<Data>& data);

    void acquireSlot ( MediaData *mediaData, FrameNumber frameNo );

    boost::shared_ptr<FrameBuffer::Slot> popSlot();


	//bool status_;

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

    mutable std::recursive_mutex syncMutex_;

    typedef boost::shared_ptr<Slot> SlotPtr;
    typedef
        priority_queue< SlotPtr, vector<SlotPtr>, Slot::Comparator>
    PlaybackQueue;

    PlaybackQueue playbackQueue_;
    std::vector<boost::shared_ptr<Slot> > freeSlots_;
    std::map<Name, boost::shared_ptr<Slot>> activeSlots_;

    IFrameBufferCallback *callback_;
};



#endif /* FRAME_BUFFER_H_ */
