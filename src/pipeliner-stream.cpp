/*
  *PipelinerStream.cpp
 *
  * Created on: Jun 15, 2016
  *     Author: xyh
 */

#include "pipeliner-stream.h"

#include "consumer.h"
#include "frame-data.h"
#include "glogger.h"
#include "name-components.h"
#include "mtndn-namespace.h"

#include <boost/thread.hpp>

//#define __SHOW_CONSOLE_

using namespace ndn::func_lib;

PipelinerStream::PipelinerStream(const Consumer *consumer):
    Pipeliner(consumer)
{
    VLOG(LOG_TRACE) << "[PipelinerStream] ctor" << endl;
//	PipelinerStreamFIle_ = fopen ( "PipelinerStreamFIle_.264", "wb+" );
//	if ( PipelinerStreamFIle_ == NULL )
//	{
//		std::cout << "open consumer.yuv error" << std::endl;
//		return;
//	}

}

PipelinerStream::~PipelinerStream()
{
//    frameBuffer_.reset();
//    faceWrapper_.reset();
    VLOG(LOG_TRACE) << "[PipelinerStream] dtor" << endl;
    //fclose(PipelinerStreamFIle_);
}


int
PipelinerStream::init()
{
    Name tmpname(consumer_->getPrefix());
    streamName_ = tmpname;
    window_.init(30/*,frameBuffer_*/);
    switchToState(StateInactive);
    interestLifeTimeMS = 5 * 1000;
    return RESULT_OK;
}

int
PipelinerStream::start()
{
    //requestMeta();
    switchToState(StateFetching);
    request();
    scheduleJob(interestLifeTimeMS * 1000 - 100,boost::bind(&PipelinerStream::request,this));
    //request();
    VLOG(LOG_INFO) << "[PipelinerStream] Started" << endl;

    return RESULT_OK;
}

int
PipelinerStream::stop()
{
    Pipeliner::stop();
}

ptr_lib::shared_ptr<Interest>
PipelinerStream::getDefaultInterest()
{
    ptr_lib::shared_ptr<Interest> interest(new Interest(streamName_));
    interest->setMustBeFresh(true);
    interest->setInterestLifetimeMilliseconds(interestLifeTimeMS);
    return interest;
}


//******************************************************************************

void
PipelinerStream::onData(const ptr_lib::shared_ptr<const Interest> &interest,
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
            lastFrmNo_ = reqCurPktNo_;
            switchToState(StateFetching);
            Interest interest(streamName_);
            interest.setInterestLifetimeMilliseconds(6*10*1000);
            express(interest);
            //requestNextPkt();
        }
        else
        {
            LOG(WARNING) << "[PipelinerStream] meta packet error: " << data->getName() << endl;
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
        //LOG(INFO) << "[PipelinerStream] Received Data " << data->getName().to_uri() << " " << reqCurPktNo_ << " " << reqLastNo_ << std::endl;
        frameBuffer_->recvData(data);
        //requestNextPkt();
        if( callback_ )
            callback_->onBufferingEnded(); // start playout
    }
        break;

    default:
    {
        VLOG(LOG_ERROR) << "[PipelinerStream] State error" << endl;
    }
        break;
    }//switch
}

void
PipelinerStream::onTimeout(const ptr_lib::shared_ptr<const Interest> &interest)
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
        VLOG(LOG_ERROR) << "[PipelinerStream] State error" << endl;
    }
        break;
    }//switch

}

//******************************************************************************

void
PipelinerStream::requestMeta()
{
    ndn::Name metaName( streamName_ );
    metaName.append(NameComponents::NameComponentStreamMetainfo);
    metaName.append(MtNdnUtils::componentFromInt(MtNdnUtils::microsecondTimestamp()));

    ndn::Interest metaInterest( metaName, 1000 );
    metaInterest.setMustBeFresh(true);

    express(metaInterest);
}

bool
PipelinerStream::request()
{
    request_count_++;
    cout << "Request: " << request_count_ << endl;
    express(*getDefaultInterest().get());
    return true;
}
