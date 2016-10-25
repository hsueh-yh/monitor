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
#include "name-components.h"
#include "namespacer.h"

#define __SHOW_CONSOLE_

using namespace ndn::func_lib;


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
    std::lock_guard<std::mutex> scopedLock(mutex_);
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
    std::lock_guard<std::mutex> scopedLock(mutex_);

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
    requestPktNo_(0),
    state_(Stoped),
    isRetransmission(false),
    statistic(Statistics::getInstance())
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
    faceWrapper_ = faceWrapper;
    window_.init(100/*,frameBuffer_*/);
    changetoState(Pipeliner::State::Ready);
}


void
Pipeliner::express(Interest& interest/*, int64_t priority*/)
{
    faceWrapper_->expressInterest(
            interest,
            func_lib::bind(&Pipeliner::onData, this, func_lib::_1, func_lib::_2),
            func_lib::bind(&Pipeliner::onTimeout, this, func_lib::_1) );
            //bind2nd(bind(&Pipeliner::onData, this, func_lib::_1, func_lib::_2),
            //bind(&Pipeliner::onTimeout, this, func_lib::_1));

#ifdef __SHOW_CONSOLE_
    int componentCount = interest.getName().getComponentCount();
    cout << "Express : " << interest.getName().get(componentCount-1).toEscapedString()
         << " " << interest.getName().toUri() << endl;
#endif
}


void
Pipeliner::express(Name& name/*, int64_t priority*/)
{
    faceWrapper_->expressInterest(
            name,
            func_lib::bind(&Pipeliner::onData, this, func_lib::_1, func_lib::_2),
            func_lib::bind(&Pipeliner::onTimeout, this, func_lib::_1) );
            //bind(&Pipeliner::onData, this, std::placeholders::_1, std::placeholders::_2),
            //bind(&Pipeliner::onTimeout, this, std::placeholders::_1));
    statistic->addRequest();
    LOG(INFO) << "Express Interest " << name.to_uri() << endl;

#ifdef __SHOW_CONSOLE_
//    time_t t = time(NULL);
//    cout << "time " << t << endl;

    int componentCount = name.getComponentCount();
    cout << "Express : " << name.get(componentCount-1).toEscapedString()
         << " " << name.toUri() << endl;
#endif
}


void
Pipeliner::requestFrame(PacketNumber frameNo)
{
    if( !window_.canAskForData(frameNo) )
    {
        //cout << "Windown: " << window_.getCurrentWindowSize() << endl;
        usleep(10*1000);
    }
    LOG(INFO) << "Request " << frameNo << endl;
    Name packetPrefix(basePrefix_);
    packetPrefix.append(NdnRtcUtils::componentFromInt(frameNo));
    //packetPrefix.appendTimestamp(NdnRtcUtils::microsecondTimestamp());
    //ptr_lib::shared_ptr<Interest> frameInterest = getDefaultInterest(packetPrefix);

//    ptr_lib::shared_ptr<FrameBuffer::Slot> slot;

//    slot.reset(new FrameBuffer::Slot());
//    slot->lock();
//    slot->setPrefix(packetPrefix);
//    slot->setNumber(frameNo);
//    slot->interestIssued();

    //frameBuffer_->lock();
    ndn::Interest interest(packetPrefix);

    while( !frameBuffer_->interestIssued(interest))
    {
        usleep(10*1000);
        //cout << ".";
    }

    //frameBuffer_->activeSlots_.insert(std::pair<int,ptr_lib::shared_ptr<FrameBuffer::Slot>>(frameNo,slot));
    //frameBuffer_->unlock();

    //slot->unlock();

    //express(frameInterest);
    express(interest);
}


void
Pipeliner::startFetching()
{
    Name packetPrefix;
    while( getState() != Stoped )
    {
        switch(state_)
        {
        case Ready:
            packetPrefix = basePrefix_;
            packetPrefix.append(NameComponents::NameComponentStreamMetainfo);
            packetPrefix.append(NdnRtcUtils::componentFromInt(NdnRtcUtils::microsecondTimestamp()));
            express(packetPrefix);
            changetoState(Bootstrap);
            break;

        case Bootstrap:
            usleep(10*1000);
            continue;
            break;

        case Fetching:
            requestFrame(requestPktNo_++);
            break;

        default:
            LOG(ERROR) << "[Pipeliner] State error" << endl;
            break;
        }

//        requestFrame(frameNo);
//        frameNo++;

        usleep(30*1000);
    }
    //cout << "[Pipeliner] stop fetching" << endl;
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
    //LOG(INFO) << "Recieve Data " << data->getName().to_uri() << endl;

    if (getState() == Stoped)
        return;


#ifdef __SHOW_CONSOLE_
    //time_t t = time(NULL);

    //cout << "time " << t << endl;

    cout << "Got Data: "<< data->getName().toUri()
         << " size: " << data->getContent ().size () << endl;

    //NdnRtcUtils::printMem("Got Data", data->getContent().buf(),data->getContent().size()==0?0:30);
#endif

    int p = Namespacer::findComponent(data->getName(), NameComponents::NameComponentStreamMetainfo );
    if( -1 != p )
    {
        requestPktNo_ = NdnRtcUtils::frameNumber(data->getName().get(p+2));
        cout << requestPktNo_ << "**********************"<<endl;
        changetoState(Fetching);
        return;
    }

    unsigned int pktNo;
    Namespacer::getFrameNumber(data->getName(),pktNo);

    window_.dataArrived(pktNo);

    if (getState() == Stoped)
        return;

    frameBuffer_->lock();

    if( frameBuffer_->stat_ == FrameBuffer::State::Stoped)
        return;

    frameBuffer_->dataArrived(data);
    frameBuffer_->unlock();

}


void
Pipeliner::onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
{
    statistic->markMiss();
    LOG(WARNING) << "Timeout " << interest->getName().to_uri()
                 << " ( Loss Rate = " << statistic->getLostRate() << " )"<< endl;

    int componentCount = interest->getName().getComponentCount();
    FrameNumber frameNo = std::atoi(interest->getName().get(componentCount-1).toEscapedString().c_str());
    window_.dataArrived(frameNo);

    frameBuffer_->lock();

    if( frameBuffer_->stat_ == FrameBuffer::State::Stoped)
        return;

    frameBuffer_->dataMissed(interest);
    frameBuffer_->unlock();

    if( isRetransmission )
    {

        int componentCount = interest->getName().getComponentCount();
        FrameNumber frameNo = std::atoi(interest->getName().get(componentCount-1).toEscapedString().c_str());
        requestFrame(frameNo);
        LOG(INFO) << "Retrans " << interest->getName().to_uri();
    }
}


