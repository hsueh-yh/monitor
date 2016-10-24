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
#include "namespacer.h"
#include "name-components.h"

#include <stdio.h>

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
    ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(mutex_);

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
    ptr_lib::lock_guard<ptr_lib::mutex> scopedLock(mutex_);

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
    fetchingFramNo_(0),
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
Pipeliner::init(ptr_lib::shared_ptr<FrameBuffer> frameBuffer, ptr_lib::shared_ptr<FaceWrapper> faceWrapper)
{
    frameBuffer_ = frameBuffer;
    frameBuffer_->init(10);
    faceWrapper_ = faceWrapper;
    window_.init(10/*,frameBuffer_*/);
    changetoState(Pipeliner::State::Ready);

    frameBuffer->init(50);
    frameBuffer->registerCallback(this);
}

void
Pipeliner::start()
{
    ndn::Name prefix;
    Namespacer::getStreamVideoPrefix(basePrefix_,prefix);
    prefix.append(NameComponents::NameComponentStreamMetainfo);
    prefix.append(NdnRtcUtils::toString(NdnRtcUtils::microsecondTimestamp()));

    //express(prefix);
    LOG(INFO)<<"[Pipeliner] Express Interest \"" << prefix.toUri() << "\"";
    faceWrapper_->expressInterest(
            prefix,
            func_lib::bind(&Pipeliner::onMetaData, this, func_lib::_1, func_lib::_2),
            func_lib::bind(&Pipeliner::onTimeout, this, func_lib::_1) );
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

void
Pipeliner::express(const Interest& interest/*, int64_t priority*/)
{
    if( frameBuffer_->interestIssued(interest))
    {
        faceWrapper_->expressInterest(
                    interest,
                    func_lib::bind(&Pipeliner::onData, this, func_lib::_1, func_lib::_2),
                    func_lib::bind(&Pipeliner::onTimeout, this, func_lib::_1) );
        LOG(INFO)<<"[Pipeliner] Express Interest \"" << interest.getName().toUri() << "\"";
    }
    else
    {
        LOG(INFO)<<"[Pipeliner] Express Interest canceled \"" << interest.getName().toUri() << "\"";
    }
}

void
Pipeliner::express(const Name& name/*, int64_t priority*/)
{
    faceWrapper_->expressInterest(
            name,
                func_lib::bind(&Pipeliner::onData, this, func_lib::_1, func_lib::_2),
                func_lib::bind(&Pipeliner::onTimeout, this, func_lib::_1) );
    LOG(INFO)<<"[Pipeliner] Express Interest \"" << name.toUri() << "\"";

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
Pipeliner::fetchingNext()
{
    //while( getState() != Stoped )
    if( getState() != Stoped )
    {
        requestFrame(fetchingFramNo_);
        ++fetchingFramNo_;

        usleep(10*1000);
    }
    //LOG(INFO)<<"[Pipeliner] Stop fetching";
}

ptr_lib::shared_ptr<Interest>
Pipeliner::getDefaultInterest(const ndn::Name &prefix, int64_t timeoutMs)
{
    ptr_lib::shared_ptr<Interest> interest(new Interest(prefix));
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

    if( fetchingFramNo_ < 0 )
    {
        onMetaData(interest,data);
        return;
    }
    ndn::Name prefix(data->getName());
    FrameNumber frameNo;
    Namespacer::getFrameNumber(prefix,frameNo);

    LOG(INFO) << "[Pipeliner] Got Data " << prefix.toUri()
              << " ( size = " << data->getContent().size() << " )"
              << endl;

    //NdnRtcUtils::printMem("PipelinerRCEV",data->getContent().buf(),data->getContent().size());

    window_.dataArrived(frameNo);

    if (getState() == Stoped)
        return;

    if( data->getContent().size() == 0 )
    {
        frameBuffer_->interestTimeout(*interest.get());
        return;
    }

    frameBuffer_->recvData(data);
    fetchingNext();
}

void
Pipeliner::onMetaData(const ptr_lib::shared_ptr<const Interest>& interest,
        const ptr_lib::shared_ptr<Data>& data)
{
    LOG(INFO) << "Got meta Data " << endl;
    Namespacer::getMetaNumber(data->getName(),fetchingFramNo_);
    //onData(interest,data);
    LOG(INFO) << "Got meta Data " << data->getName().toUri()
              << " frameNo = " << fetchingFramNo_ << endl;
    fetchingNext();
}

void
Pipeliner::onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
{
    LOG(INFO) << "[Pipeliner] TimeOut " << interest->getName().toUri() << endl;

    ndn::Name prefix(interest->getName());
    FrameNumber frameNo;
    Namespacer::getFrameNumber(prefix,frameNo);
    window_.dataArrived(frameNo);
    //requestFrame(frameNo);

    frameBuffer_->interestTimeout(*interest.get());
    //window_.changeWindow(-1);
}

void
Pipeliner::onSegmentNeeded( const FrameNumber frameNo, const SegmentNumber segNo )
{
    //
}

void
Pipeliner::onSegmentNeeded( const Name prefix )
{
    express(prefix);
}
