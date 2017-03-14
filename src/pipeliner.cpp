/*
  *pipeliner.cpp
 *
  * Created on: Jun 15, 2016
  *     Author: xyh
 */

#include <iostream>
#include "pipeliner.h"
#include <unistd.h>

#include "pipeliner.h"
#include "consumer.h"
#include "frame-data.h"
#include "glogger.h"
#include "name-components.h"
#include "mtndn-namespace.h"

#include <boost/thread.hpp>

//#define __SHOW_CONSOLE_

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
PipelinerWindow::init(unsigned int windowSize/*, const FrameBuffer *frameBuffer*/)
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

Pipeliner::Pipeliner(const Consumer *consumer):
    consumer_(const_cast<Consumer*>(consumer)),
    frameBuffer_(consumer_->getFrameBuffer().get()),
    face_(consumer_->getFace()),
    //streamName_(consumer_->getPrefix()),
    request_count_(0),
    reqCurPktNo_(0),
    state_(StateInactive),
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
    VLOG(LOG_TRACE) << "[Pipeliner] dtor" << endl;
    //fclose(pipelinerFIle_);
}


int
Pipeliner::init()
{
    Name tmpname(consumer_->getPrefix());
    streamName_ = tmpname;
    window_.init(30/*,frameBuffer_*/);
    switchToState(StateInactive);
    return RESULT_OK;
    VLOG(LOG_TRACE) << "Pipeliner initialized: " << streamName_.to_uri() << std::endl;
}

int
Pipeliner::start()
{
    //requestMeta();
    switchToState(StateFetching);
    unsigned int delayMs = 5*1000;
    request(delayMs);
    scheduleJob((delayMs-1)*1000,boost::bind(&Pipeliner::request,this,delayMs));
    //request();
    VLOG(LOG_INFO) << "[Pipeliner] Started" << endl;

    return RESULT_OK;
}

int
Pipeliner::stop()
{
    switchToState(StateInactive);
    window_.reset();
    VLOG(LOG_INFO) << "[Pipeliner] Stopped" << endl;
    return RESULT_OK;
}

void
Pipeliner::express(const Interest &interest/*, int64_t priority*/)
{
    face_->expressInterest(
            interest,
            func_lib::bind(&Pipeliner::onData, this, func_lib::_1, func_lib::_2),
            func_lib::bind(&Pipeliner::onTimeout, this, func_lib::_1) );
            //bind2nd(bind(&Pipeliner::onData, this, func_lib::_1, func_lib::_2),
            //bind(&Pipeliner::onTimeout, this, func_lib::_1));

    statistic->addRequest();
    LogTraceC << "Express " << interest.getName().to_uri() << std::endl;
    //VLOG(LOG_TRACE) << "Express Interest " << interest.getName().to_uri() << endl;

#ifdef __SHOW_CONSOLE_
    cout << "Express : " << interest.getName().toUri() << endl;
#endif
}

void
Pipeliner::express(const Name &name/*, int64_t priority*/)
{
    face_->expressInterest(
            name,
            func_lib::bind(&Pipeliner::onData, this, func_lib::_1, func_lib::_2),
            func_lib::bind(&Pipeliner::onTimeout, this, func_lib::_1) );
            //bind(&Pipeliner::onData, this, std::placeholders::_1, std::placeholders::_2),
            //bind(&Pipeliner::onTimeout, this, std::placeholders::_1));
    statistic->addRequest();
    LogTraceC << "Express " << name.to_uri() << std::endl;
    //VLOG(LOG_TRACE) << "Express Interest " << name.to_uri() << endl;

#ifdef __SHOW_CONSOLE_
//    time_t t = time(NULL);
//    cout << "time " << t << endl;

    int componentCount = name.getComponentCount();
    cout << "Express : " << name.get(componentCount-1).toEscapedString()
         << " " << name.toUri() << endl;
#endif
}

bool
Pipeliner::requestFrame(PacketNumber frameNo, bool isExpressInterest)
{
    if( !window_.canAskForData(frameNo) )
    {
        //cout << "Windown: " << window_.getCurrentWindowSize() << endl;
        //usleep(10*1000);
        //std::cout << "request frame1 " << window_.getCurrentWindowSize() << std::endl;
        return false;
    }
    //LOG(INFO) << "Request " << frameNo << endl;
    Name packetPrefix(streamName_);
    packetPrefix.append(MtNdnUtils::componentFromInt(frameNo));
    //packetPrefix.appendTimestamp(MtNdnUtils::microsecondTimestamp());
    //ptr_lib::shared_ptr<Interest> frameInterest = getDefaultInterest(packetPrefix);

//    ptr_lib::shared_ptr<FrameBuffer::Slot> slot;

//    slot.reset(new FrameBuffer::Slot());
//    slot->lock();
//    slot->setPrefix(packetPrefix);
//    slot->setNumber(frameNo);
//    slot->interestIssued();

    //frameBuffer_->lock();
    ndn::Interest interest(packetPrefix);
    interest.setInterestLifetimeMilliseconds(2*10*1000);

    if( !frameBuffer_->interestIssued(interest))
    {
        //usleep(10*1000);
        //cout << ".";

        LOG(ERROR) << "FrameBuffer::interestIssued false " << std::endl;
    }

    //frameBuffer_->activeSlots_.insert(std::pair<int,ptr_lib::shared_ptr<FrameBuffer::Slot>>(frameNo,slot));
    //frameBuffer_->unlock();

    //slot->unlock();

    //express(frameInterest);
    if(isExpressInterest)
        express(interest);
    return true;
}

ptr_lib::shared_ptr<Interest>
Pipeliner::getDefaultInterest(const ndn::Name &prefix)
{
    ptr_lib::shared_ptr<Interest> interest(new Interest(prefix));
    interest->setMustBeFresh(true);

    return interest;
}


//******************************************************************************

void
Pipeliner::onData(const ptr_lib::shared_ptr<const Interest> &interest,
        const ptr_lib::shared_ptr<Data> &data)
{
    //LOG(INFO) << "Recieve Data " << data->getName().to_uri() << endl;
    LogTraceC << "Recieve Data " << data->getName().to_uri() << std::endl;

#ifdef __SHOW_CONSOLE_
    cout << "Got Data: "<< data->getName().toUri()
         << " size: " << data->getContent ().size () << endl;
#endif

    switch(state_)
    {
    case StateInactive:
        break;

    //Receive MetaData
    case StateWaitInitial:
    {
        int p = Namespacer::findComponent(data->getName(), NameComponents::NameComponentStreamMetainfo );
        if( -1 != p )
        {
            reqCurPktNo_ = MtNdnUtils::frameNumber(data->getName().get(p+2));
            //cout << reqCurPktNo_ << "**********************"<<endl;
            reqLastNo_ = reqCurPktNo_;
            switchToState(StateFetching);
            Interest interest(streamName_);
            interest.setInterestLifetimeMilliseconds(6*10*1000);
            express(interest);
            //requestNextPkt();
        }
        else
        {
            LOG(WARNING) << "[Pipeliner] meta packet error: " << data->getName() << endl;
            requestMeta();
        }
    }
        break;

    case StateBootstrap:
    case StateFetching:
    {
        reqLastNo_ = MtNdnUtils::frameNumber(data->getName().get(-1));
        unsigned int pktNo;
        Namespacer::getFrameNumber(data->getName(),pktNo);

        window_.dataArrived(pktNo);

        if( frameBuffer_->getState() == FrameBuffer::Invalid)
            return;
        //LOG(INFO) << "[Pipeliner] Received Data " << data->getName().to_uri() << " " << reqCurPktNo_ << " " << reqLastNo_ << std::endl;
        frameBuffer_->recvData(data);
        //requestNextPkt();
        if( callback_)
            callback_->onBufferingEnded();
    }
        break;

    default:
    {
        VLOG(LOG_ERROR) << "[Pipeliner] State error" << endl;
    }
        break;
    }//switch
}

void
Pipeliner::onTimeout(const ptr_lib::shared_ptr<const Interest> &interest)
{
    statistic->markMiss();
    VLOG(LOG_INFO) << "Timeout " << interest->getName().to_uri()
                 << " ( Loss Rate = " << statistic->getLostRate() << " )"<< endl;

    PacketNumber pktNo;
    Namespacer::getFrameNumber(interest->getName(), pktNo);
    window_.dataArrived(pktNo);

    switch(state_)
    {
    case StateInactive:
    case StateWaitInitial:
    {
        //requestMeta();
    }
        break;

    case StateBootstrap:
    case StateFetching:
    {
        if( frameBuffer_->getState() == FrameBuffer::Invalid)
            return;
        //frameBuffer_->dataMissed(interest);
        //express(*(interest.get()));
    }
        break;

    default:
    {
        VLOG(LOG_ERROR) << "[Pipeliner] State error" << endl;
    }
        break;
    }//switch

}

void
Pipeliner::requestMeta()
{
    ndn::Name metaName( streamName_ );
    metaName.append(NameComponents::NameComponentStreamMetainfo);
    metaName.append(MtNdnUtils::componentFromInt(MtNdnUtils::microsecondTimestamp()));

    ndn::Interest metaInterest( metaName, 1000 );
    metaInterest.setMustBeFresh(true);

    express(metaInterest);
}

void
Pipeliner::requestNextPkt()
{
    if( reqCurPktNo_ == reqLastNo_+1 )
    {
        //usleep(30*1000);
        scheduleJob(30*1000, [this]()->bool{
            bool res = requestFrame(reqCurPktNo_++);
            return (!res); // if not requeste, do it again
        });
        //scheduleJob(30*1000, bind(&Pipeliner::requestFrame, this, reqCurPktNo_));
        //++reqCurPktNo_;
        //requestFrame(reqCurPktNo_++);
    }
    else
    {
        while( reqCurPktNo_ <= reqLastNo_ )
            requestFrame(reqCurPktNo_++);
    }

    if( state_ >= StateFetching && frameBuffer_->getPlayableBufferSize() >= 10 )
    {
        if( callback_)
            callback_->onBufferingEnded();
    }
}

bool
Pipeliner::request(unsigned int delay)
{
    request_count_++;
    cout << "Request: " << request_count_ << endl;
    Interest interest(streamName_);
    interest.setInterestLifetimeMilliseconds(delay);
    express(interest);
    return true;
}
