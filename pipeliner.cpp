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
    cout << "Pipeliner init " << w_ << endl;
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

	pipelinerFIle_ = fopen ( "pipelinerFIle_.264", "wb+" );
	if ( pipelinerFIle_ == NULL )
	{
		std::cout << "open consumer.yuv error" << std::endl;
		return;
	}

}


Pipeliner::~Pipeliner()
{
    cout << "Pipeliner dtor" << endl;
	fclose(pipelinerFIle_);
}


void
Pipeliner::init(boost::shared_ptr<FrameBuffer> frameBuffer, boost::shared_ptr<FaceWrapper> faceWrapper)
{
    frameBuffer_ = frameBuffer;
    faceWrapper_ = faceWrapper;
    window_.init(100/*,frameBuffer_*/);
    state_ = Ready;
}


void
Pipeliner::express(Interest& interest/*, int64_t priority*/)
{
    faceWrapper_->expressInterest(
            interest,
            bind(&Pipeliner::onData, this, _1, _2),
            bind(&Pipeliner::onTimeout, this, _1));

#ifdef __SHOW_CONSOLE_
    cout << "Express : " << interest.getName().toUri() << endl;
#endif
}


void
Pipeliner::express(Name& name/*, int64_t priority*/)
{
    faceWrapper_->expressInterest(
            name,
            bind(&Pipeliner::onData, this, _1, _2),
            bind(&Pipeliner::onTimeout, this, _1));

#ifdef __SHOW_CONSOLE_
    cout << "Express : " << name.toUri() << endl;
#endif
}


void
Pipeliner::requestFrame(PacketNumber& frameNo)
{
    if( !window_.canAskForData(frameNo) )
    {
        //cout << "Windown: " << window_.getCurrentWindowSize() << endl;
        usleep(100000);
    }

    Name packetPrefix(basePrefix_);
    packetPrefix.append(NdnRtcUtils::componentFromInt(frameNo));
    //packetPrefix.appendTimestamp(NdnRtcUtils::microsecondTimestamp());
    //boost::shared_ptr<Interest> frameInterest = getDefaultInterest(packetPrefix);

    boost::shared_ptr<FrameBuffer::Slot> slot;

    slot.reset(new FrameBuffer::Slot());
    slot->lock();
    slot->setPrefix(packetPrefix);
    slot->setNumber(frameNo);
    slot->interestIssued();

    //frameBuffer_->lock();
    while( !frameBuffer_->pushSlot(slot))
    {
        usleep(100000);
        //cout << ".";
    }

    frameBuffer_->activeSlots_.insert(std::pair<int,boost::shared_ptr<FrameBuffer::Slot>>(frameNo,slot));
    //frameBuffer_->unlock();

    slot->unlock();

    //express(frameInterest);
    express(packetPrefix);
}


void
Pipeliner::startFeching()
{
    int frameNo = 0;
    while(1)
    {
        if(state_ == Stoped)
            break;
        requestFrame(frameNo);
        frameNo++;
    }
}


void
Pipeliner::stop()
{
    if(state_ != Stoped)
        state_ != Stoped;
    return;
}


boost::shared_ptr<Interest>
Pipeliner::getDefaultInterest(const ndn::Name &prefix, int64_t timeoutMs)
{
    boost::shared_ptr<Interest> interest(new Interest(prefix));
    interest->setMustBeFresh(true);

    return interest;
}



//******************************************************************************
//******************************************************************************

void
Pipeliner::onData(const ptr_lib::shared_ptr<const Interest>& interest,
		const ptr_lib::shared_ptr<Data>& data)
{
    FrameNumber frameNo = std::atoi(data->getName().get(1).toEscapedString().c_str());
	//std::cout<<"Pipeliner onData:"<<std::endl;

//	if ( data->getContent ().buf () == NULL )
//			cout << "content is null !" << endl;
//    cout << "Pipeliner:" << (int)getpid() << "-" << std::this_thread::get_id() << " ";

#ifdef __SHOW_CONSOLE_
    cout << "Got Data: "<<frameNo// << " "<< data->getName().toUri()
         << " size: " << data->getContent ().size () << endl;
#endif

//    FrameData gotFrame;
//    memcpy(gotFrame, data->getContent().buf(), sizeof(FrameData));  //copy frame header

//    gotFrame.buf_ = (unsigned char*) malloc (gotFrame.header_.length_);
//    memcpy( gotFrame.buf_, data->getContent().buf()+sizeof(FrameData), gotFrame.header_.length_);   //copy frame data

    window_.dataArrived(frameNo);
    frameBuffer_->lock();
    frameBuffer_->dataArrived(data);
    frameBuffer_->unlock();

}


void
Pipeliner::onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
{

#ifdef __SHOW_CONSOLE_
    cout << "Pipeliner timeout: " << interest->getName().to_uri()<< endl;
#endif

}


