/*
  *PipelinerFrame.cpp
 *
  * Created on: Jun 15, 2016
  *     Author: xyh
 */

#include "pipeliner-frame.h"

#include "consumer.h"
#include "frame-data.h"
#include "glogger.h"
#include "name-components.h"
#include "mtndn-namespace.h"

#include <boost/thread.hpp>
//#define __SHOW_CONSOLE_

using namespace ndn::func_lib;


PipelinerFrame::PipelinerFrame(const Consumer *consumer):
    Pipeliner(consumer)
{
    VLOG(LOG_TRACE) << "[PipelinerFrame] ctor" << endl;
//	PipelinerFrameFIle_ = fopen ( "PipelinerFrameFIle_.264", "wb+" );
//	if ( PipelinerFrameFIle_ == NULL )
//	{
//		std::cout << "open consumer.yuv error" << std::endl;
//		return;
//	}

}

PipelinerFrame::~PipelinerFrame()
{
//    frameBuffer_.reset();
//    faceWrapper_.reset();
    VLOG(LOG_TRACE) << "[PipelinerFrame] dtor" << endl;
    //fclose(PipelinerFrameFIle_);
}


int
PipelinerFrame::init()
{
    Name tmpname(consumer_->getPrefix());
    streamName_ = tmpname;
    window_.init(30/*,frameBuffer_*/);
    switchToState(StateInactive);
    return RESULT_OK;
}

int
PipelinerFrame::start()
{
    requestMeta();
    switchToState(StateWaitInitial);

    VLOG(LOG_INFO) << "[PipelinerFrame] Started" << endl;

    return RESULT_OK;
}

int
PipelinerFrame::stop()
{
    Pipeliner::stop();
}

ptr_lib::shared_ptr<Interest>
PipelinerFrame::getDefaultInterest(PacketNumber frameNo)
{
    Name packetPrefix(streamName_);
    packetPrefix.append(MtNdnUtils::componentFromInt(frameNo));
    ptr_lib::shared_ptr<Interest> interest(new Interest(packetPrefix));
    interest->setInterestLifetimeMilliseconds(1*1000);
    interest->setMustBeFresh(true);

    return interest;
}


//******************************************************************************

void
PipelinerFrame::onData(const ptr_lib::shared_ptr<const Interest> &interest,
        const ptr_lib::shared_ptr<Data> &data)
{
    //LOG(INFO) << "Recieve Data " << data->getName().to_uri() << endl;
    //LogTraceC << "Recieve Data " << data->getName().to_uri() << std::endl;

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
            lastFrmNo_ = reqCurPktNo_;
            switchToState(StateFetching);
            requestNextPkt();
        }
        else
        {
            LOG(WARNING) << "[PipelinerFrame] meta packet error: " << data->getName() << endl;
            requestMeta();
        }
    }
        break;

    case StateBootstrap:
    case StateFetching:
    {
        lastFrmNo_ = MtNdnUtils::frameNumber(data->getName().get(-1));
        unsigned int pktNo;
        Namespacer::getFrameNumber(data->getName(),pktNo);

        window_.dataArrived(pktNo);

        if( frameBuffer_->getState() == FrameBuffer::Invalid)
            return;
        //LOG(INFO) << "[PipelinerFrame] Received Data " << data->getName().to_uri() << " " << reqCurPktNo_ << " " << reqLastNo_ << std::endl;
        frameBuffer_->recvData(data);
        requestNextPkt();
    }
        break;

    default:
    {
        VLOG(LOG_ERROR) << "[PipelinerFrame] State error" << endl;
    }
        break;
    }//switch
}

void
PipelinerFrame::onTimeout(const ptr_lib::shared_ptr<const Interest> &interest)
{
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
        requestMeta();
    }
        break;

    case StateBootstrap:
    case StateFetching:
    {
        if( frameBuffer_->getState() == FrameBuffer::Invalid)
            return;
        frameBuffer_->dataMissed(interest);
        VLOG(LOG_TRACE) << "RE-Express " << interest->getName().to_uri() << endl;
        express(*(interest.get()));
    }
        break;

    default:
    {
        VLOG(LOG_ERROR) << "[PipelinerFrame] State error" << endl;
    }
        break;
    }//switch
}

//******************************************************************************

void
PipelinerFrame::requestMeta()
{
    ndn::Name metaName( streamName_ );
    metaName.append(NameComponents::NameComponentStreamMetainfo);
    metaName.append(MtNdnUtils::componentFromInt(MtNdnUtils::microsecondTimestamp()));

    ndn::Interest metaInterest( metaName, 1000 );
    metaInterest.setMustBeFresh(true);

    express(metaInterest);
}

void
PipelinerFrame::requestNextPkt()
{
    if( reqCurPktNo_ >= lastFrmNo_ )
    {
        //usleep(30*1000);
        cout << "sleep 30 ms " << endl << endl;
        scheduleJob(30*1000, [this]()->bool{
            bool res = requestFrame(reqCurPktNo_++);
            return (!res); // if not requeste, do it again
        });
        //scheduleJob(30*1000, bind(&PipelinerFrame::requestFrame, this, reqCurPktNo_));
        //++reqCurPktNo_;
        //requestFrame(reqCurPktNo_++);
    }
    else
    {
        while( reqCurPktNo_ <= lastFrmNo_ )
            requestFrame(reqCurPktNo_++);
    }

    if( state_ >= StateFetching && frameBuffer_->getPlayableBufferSize() >= 3 )
    {
        if( callback_)
            callback_->onBufferingEnded();
    }
}

bool
PipelinerFrame::requestFrame(PacketNumber frameNo)
{
    if( !window_.canAskForData(frameNo) )
    {
        //cout << "Windown: " << window_.getCurrentWindowSize() << endl;
        //usleep(10*1000);
        //std::cout << "request frame1 " << window_.getCurrentWindowSize() << std::endl;
        return false;
    }
    //LOG(INFO) << "Request " << frameNo << endl;

    ptr_lib::shared_ptr<Interest> interest = getDefaultInterest( frameNo );

    if( !frameBuffer_->interestIssued(*interest.get()))
    {
        LOG(ERROR) << "FrameBuffer::interestIssued false " << std::endl;
    }

    express(*interest.get());
    return true;
}
