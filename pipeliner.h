/*
 * Pipeliner.h
 *
 *  Created on: Jun 15, 2016
 *      Author: xyh
 */

#ifndef PIPELINER_H_
#define PIPELINER_H_

#include <ndn-cpp/name.hpp>
#include <ndn-cpp/data.hpp>

#include "common.h"
#include "frame-buffer.h"
#include "face-wrapper.h"

using namespace ndn;


class PipelinerWindow
{
public:
    PipelinerWindow();
    ~PipelinerWindow();

    void
    init(unsigned int windowSize/*, const FrameBuffer* frameBuffer*/);

    void
    reset();

    void
    dataArrived(PacketNumber packetNo);

    bool
    canAskForData(PacketNumber packetNo);

    unsigned int
    getDefaultWindowSize();

    int
    getCurrentWindowSize();

    int
    changeWindow(int delta);

    bool
    isInitialized()
    { return isInitialized_; }

private:
    unsigned int dw_;   // default window size
    int w_;             // current window size
    bool isInitialized_;
    PacketNumber lastAddedToPool_;
    boost::mutex mutex_;
    std::set<PacketNumber> framePool_;
    const FrameBuffer* frameBuffer_;
};


class Pipeliner
{
public:

    Pipeliner(boost::shared_ptr<FrameBuffer> frameBuffer, boost::shared_ptr<FaceWrapper> faceWrapper );
	~Pipeliner();

    //******************************************************************************

    void express(Interest& interest);
    void express(Name& name);

    void requestFrame(PacketNumber& frameNo);

    void startFeching();

    boost::shared_ptr<Interest>
    getDefaultInterest(const Name& prefix, int64_t timeoutMs = 0);

    //******************************************************************************
	void onData(const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& data);

	void onTimeout(const ptr_lib::shared_ptr<const Interest>& interest);
/*
	OnData
	getOnDataHandler()
	{ return bind(&Pipeliner::onData, boost::dynamic_pointer_cast<Pipeliner>(shared_from_this()), _1, _2); }

	OnTimeout
	getOnTimeoutHandler()
	{ return bind(&Pipeliner::onTimeout, boost::dynamic_pointer_cast<Pipeliner>(shared_from_this()), _1); }
*/

private:

    Name basePrefix_;

    boost::shared_ptr<FaceWrapper> faceWrapper_;
    boost::shared_ptr<FrameBuffer> frameBuffer_;
    PipelinerWindow window_;

    FILE *pipelinerFIle_;

    int count_;
};


#endif /* PIPELINER_H_ */
