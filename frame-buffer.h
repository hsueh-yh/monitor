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

using namespace std;
using namespace ndn;


class FrameBuffer
{
public:

    typedef enum{
        Stoped = -1,
        Ready = 0,
        Started = 1
    }State;

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
             * Discards segment by swithcing it to NotUsed state and
             * reseting all the attributes
             */
            void discard();

            /**
             * Moves segment into Pending state and updaets following
             * attributes:
             * - requestTimeUsec
             * - interestName
             * - interestNonce
             * - reqCounter
             */
            void interestIssued(const uint32_t& nonceValue);

            /**
             * Moves segment into Missing state if it was in Pending
             * state before
             */
            void markMissed();

            /**
             * Moves segment into Fetched state and updates following
             * attributes:
             * - dataName
             * - dataNonce
             * - generationDelay
             * - arrivalTimeUsec
             */
            void
            dataArrived(const SegmentData::SegmentMetaInfo& segmentMeta);

            /**
             * Returns true if the interest issued for a segment was
             * answered by a producer
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
            Name prefix_;
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

            void resetData();

        }; //class Segment


        enum State{
            StateFetched = 1<<0,    // frame has been fetched already
            StatePending = 1<<1,    // frame awaits it's interest to
                                    // be answered
            StateMissing = 1<<2,    // frame was timed out or
                                    // interests has not been issued yet
            StateNotUsed = 1<<3     // frame is no used
        };

		class Comparator
		{
		public:
			Comparator(bool inverted = false):inverted_(inverted){}

            bool operator() (const boost::shared_ptr<Slot> slot1, const boost::shared_ptr<Slot> slot2)
			{
                return slot1->getNumber() > slot2->getNumber();
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
         * Discards frame by swithcing it to NotUsed state and
         * reseting all the attributes
         */
        void discard();

        /**
         * Moves frame into Pending state and updaets following
         * attributes:
         * - requestTimeUsec
         * - interestName
         * - reqCounter
         */
        void interestIssued();

        /**
         * Moves frame into Missing state if it was in Pending
         * state before
         */
        void markMissed();

        /**
         * Moves frame into Fetched state and updates following
         * attributes:
         * - dataName
         * - arrivalTimeUsec
         */
        void
        dataArrived();

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
        getNumber() const { return frameNumber_; }

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

        FrameNumber frameNumber_;
        Name prefix_;

        unsigned int payloadSize_;  // size of actual data payload
                                    // (without frame header)
        unsigned char* dataPtr_;    // pointer to the payload data

        State state_;

        int64_t requestTimeUsec_, // local timestamp when the interest
                                  // for this frame was issued
                arrivalTimeUsec_; // local timestamp when data for this
                                  // frame has arrived
        std::recursive_mutex syncMutex_;

        void resetData();

    };// class Slot///////////////////////////////////////////////////////////////


	FrameBuffer():
        count_(0),
        activeSlots_count_(0),
        stat_(Started)
	{}


    ~FrameBuffer()
    {
#ifdef __SHOW_CONSOLE_
        cout << "[FrameBuffer] dtor" << endl;
#endif
    }

    void
    init()
	{
        //stat_ = Ready;
	}

    void stop()
    {
        stat_ = Stoped;
    }

    void
    lock()  { syncMutex_.lock(); }

    void
    unlock() { syncMutex_.unlock(); }

    bool pushSlot(boost::shared_ptr<Slot> slot);

    void setSlot(const ndn::ptr_lib::shared_ptr<Data>& data, boost::shared_ptr<Slot> slot);

    void dataArrived(const ndn::ptr_lib::shared_ptr<Data>& data);

    boost::shared_ptr<FrameBuffer::Slot> popSlot();


	//bool status_;

//private:

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

    typedef boost::shared_ptr<Slot> SlotPtr;

    typedef
        priority_queue< SlotPtr, vector<SlotPtr>, Slot::Comparator/*greater<Slot::Comparator>*/ >
    PlaybackQueue;

	int count_;

    State stat_;

    //PriorityQueue priorityQueue_;
    //std::recursive_mutex syncMutex_;
    mutable boost::recursive_mutex syncMutex_;


    //std::vector<boost::shared_ptr<Slot> > issuedSlots_;
    std::map<int, boost::shared_ptr<Slot> > activeSlots_;
    PlaybackQueue playbackQueue_;

    int activeSlots_count_;

    double playbackRate = 30.0;
};


#endif /* FRAME_BUFFER_H_ */
