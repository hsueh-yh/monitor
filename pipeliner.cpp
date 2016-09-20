/*
 * pipeliner.cpp
 *
 *  Created on: Jun 15, 2016
 *      Author: xyh
 */

#include <iostream>
#include "pipeliner.h"
#include <unistd.h>

#include "pipeliner.h"
#include "consumer.h"
#include "frame-data.h"
#include "logger.hpp"

using namespace std;


/////////////////////////////////////////////////////////////////////////////////
///     PipelinerWindow
/////////////////////////////////////////////////////////////////////////////////

PipelinerWindow::PipelinerWindow():
    isInitialized_(false)
{
}

PipelinerWindow::~PipelinerWindow()
{
}

void
PipelinerWindow::init(unsigned int windowSize/*, const FrameBuffer* frameBuffer*/)
{
    isInitialized_ = true;
    //frameBuffer_ = frameBuffer;
    framePool_.clear();
    dw_ = windowSize;
    w_ = (int)windowSize;
}

void
PipelinerWindow::reset()
{
    isInitialized_ = false;
}

void
PipelinerWindow::dataArrived(PacketNumber packetNo)
{
    boost::lock_guard<boost::mutex> scopedLock(mutex_);

    std::set<PacketNumber>::iterator it = framePool_.find(packetNo);

    if (it != framePool_.end())
    {
        w_++;
        framePool_.erase(it);
    }
}

bool
PipelinerWindow::canAskForData(PacketNumber packetNo)
{
    boost::lock_guard<boost::mutex> scopedLock(mutex_);

    bool added = false;
    if (w_ > 0)
    {
        w_--;
        framePool_.insert(packetNo);
        lastAddedToPool_ = packetNo;
        added = true;
    }
    return added;
}

unsigned int
PipelinerWindow::getDefaultWindowSize()
{
    return dw_;
}

int
PipelinerWindow::getCurrentWindowSize()
{
    return w_;
}

int
PipelinerWindow::changeWindow(int delta)
{
    w_ += delta;
    /*
    unsigned int nOutstandingFrames = frameBuffer_->getNewSlotsNum();

    if (delta < 0 &&
        abs(delta) >= nOutstandingFrames &&
        nOutstandingFrames > 1)
        delta = -(nOutstandingFrames-1);

    if (dw_+delta > 0)
    {
        lock_guard<mutex> scopedLock(mutex_);
        dw_ += delta;
        w_ += delta;

        return delta;
    }
    */

    return 0;
}


/////////////////////////////////////////////////////////////////////////////////
///     Pipeliner
/////////////////////////////////////////////////////////////////////////////////

Pipeliner::Pipeliner(std::string prefix):
    basePrefix_(prefix.c_str()),
    count_(0),
    state_(Stoped)
{

//	pipelinerFIle_ = fopen ( "pipelinerFIle_.264", "wb+" );
//	if ( pipelinerFIle_ == NULL )
//	{
//		std::cout << "open consumer.yuv error" << std::endl;
//		return;
//	}

}

Pipeliner::~Pipeliner()
{
//    frameBuffer_.reset();
//    faceWrapper_.reset();
#ifdef __SHOW_CONSOLE_
    cout << "[Pipeliner] dtor" << endl;
#endif
    //fclose(pipelinerFIle_);
}

void
Pipeliner::init(boost::shared_ptr<FrameBuffer> frameBuffer, boost::shared_ptr<FaceWrapper> faceWrapper)
{
    frameBuffer_ = frameBuffer;
    faceWrapper_ = faceWrapper;
    window_.init(50/*,frameBuffer_*/);
    changetoState(Pipeliner::State::Ready);

    frameBuffer->init(50);
    frameBuffer->registerCallback(this);
}

void
Pipeliner::express(Interest& interest/*, int64_t priority*/)
{
    if( frameBuffer_->interestIssued(interest))
    {
        faceWrapper_->expressInterest(
                    interest,
                    bind(&Pipeliner::onData, this, _1, _2),
                    bind(&Pipeliner::onTimeout, this, _1));
        LOG(INFO)<<"[Pipeliner] Express Interest \"" << interest.getName() << "\"";
    }
    else
    {
        LOG(INFO)<<"[Pipeliner] Express Interest canceled \"" << interest.getName() << "\"";
    }
}

void
Pipeliner::express(Name& name/*, int64_t priority*/)
{
    faceWrapper_->expressInterest(
            name,
            bind(&Pipeliner::onData, this, _1, _2),
            bind(&Pipeliner::onTimeout, this, _1));

#ifdef __SHOW_CONSOLE_
    time_t t = time(NULL);
    cout << "time " << t << endl;

    int componentCount = name.getComponentCount();
    cout << "Express : " << name.get(componentCount-1).toEscapedString()
         << " " << name.toUri() << endl;
#endif
}

void
Pipeliner::requestFrame(PacketNumber& frameNo)
{
    while( !window_.canAskForData(frameNo) )
    {
        //cout << "Windown: " << window_.getCurrentWindowSize() << endl;
        usleep(10*1000);
    }

    LOG(INFO) << "Fetching frame " << frameNo;
    Name packetPrefix(basePrefix_);
    packetPrefix.append(NdnRtcUtils::componentFromInt(frameNo));
    int segNo = estimateSegmentNumber();
    uint32_t nonce;
    for ( int i = 0; i < segNo; i++ )
    {
        Name segPrefix(packetPrefix);
        segPrefix.append(NdnRtcUtils::componentFromInt(i));
        nonce = NdnRtcUtils::generateNonceValue();
        Interest interest(segPrefix);
        interest.setNonce(NdnRtcUtils::nonceToBlob(nonce));
        express(interest);
    }
}

void
Pipeliner::startFetching()
{
    LOG(INFO)<<"[Pipeliner] Start fetching";
    int frameNo = 0;
    while( getState() != Stoped )
    {
        requestFrame(frameNo);
        frameNo++;

        usleep(10*1000);
    }
    LOG(INFO)<<"[Pipeliner] Stop fetching";
}

void
Pipeliner::stop()
{
#ifdef __SHOW_CONSOLE_
    cout << "[Pipeliner] Stoping" << endl;
#endif
    window_.reset();
    while(getState() != Stoped)
    {
        changetoState(Stoped);
    }
}

boost::shared_ptr<Interest>
Pipeliner::getDefaultInterest(const ndn::Name &prefix, int64_t timeoutMs)
{
    boost::shared_ptr<Interest> interest(new Interest(prefix));
    interest->setMustBeFresh(true);

    return interest;
}

void
Pipeliner::changetoState(Pipeliner::State stat)
{
    lock();
    state_ = stat;
    unlock();
}

//******************************************************************************
//******************************************************************************

void
Pipeliner::onData(const ptr_lib::shared_ptr<const Interest>& interest,
		const ptr_lib::shared_ptr<Data>& data)
{
    if (getState() == Stoped)
        return;
    int componentCount = data->getName().getComponentCount();
    FrameNumber frameNo = std::atoi(data->getName().get(componentCount-1).toEscapedString().c_str());
	//std::cout<<"Pipeliner onData:"<<std::endl;

//	if ( data->getContent ().buf () == NULL )
//			cout << "content is null !" << endl;
//    cout << "Pipeliner:" << (int)getpid() << "-" << std::this_thread::get_id() << " ";

#ifdef __SHOW_CONSOLE_
    //time_t t = time(NULL);

    //cout << "time " << t << endl;

    cout << "Got Data: "<<frameNo// << " "<< data->getName().toUri()
         << " size: " << data->getContent ().size () << endl;
#endif

//    FrameData gotFrame;
//    memcpy(gotFrame, data->getContent().buf(), sizeof(FrameData));  //copy frame header

//    gotFrame.buf_ = (unsigned char*) malloc (gotFrame.header_.length_);
//    memcpy( gotFrame.buf_, data->getContent().buf()+sizeof(FrameData), gotFrame.header_.length_);   //copy frame data

    window_.dataArrived(frameNo);

    if (getState() == Stoped)
        return;

    frameBuffer_->recvData(data);

}

void
Pipeliner::onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
{

#ifdef __SHOW_CONSOLE_
    cout << "Pipeliner timeout: " << interest->getName().toUri()<< endl;
#endif

    int componentCount = interest->getName().getComponentCount();
    FrameNumber frameNo = std::atoi(interest->getName().get(componentCount-1).toEscapedString().c_str());

    frameBuffer_->interestTimeout(*interest.get());
    //window_.changeWindow(-1);
}

void
Pipeliner::onSegmentNeeded( const FrameNumber frameNo, const SegmentNumber segNo )
{
    //
}
