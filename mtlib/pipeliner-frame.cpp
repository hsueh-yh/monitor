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
    setDescription("[PipelinerFrame]\t");
    VLOG(LOG_TRACE) << setw(20) << setfill(' ') << std::right << getDescription()
                    << " ctor" << endl;
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
    VLOG(LOG_TRACE) << setw(20) << setfill(' ') << std::right << getDescription()
                    << "dtor" << endl;
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

    VLOG(LOG_INFO) << "\t[PipelinerFrame]\t Started" << endl;

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
    interest->setInterestLifetimeMilliseconds(1000);
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
    MtNdnUtils::printMem("onData", data->getContent().buf(), 20 );


    switch(state_)
    {
    case StateInactive:
        break;

    //Receive MetaData
    case StateWaitInitial:
    {
        int p = Namespacer::findComponent(data->getName(),
                                          NameComponents::NameComponentStreamMetainfo );
        if( -1 != p )
        {
            VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                           << "RCV Meta: " << data->getName().toUri() << endl;
            reqCurPktNo_ = MtNdnUtils::frameNumber(data->getName().get(p+2));
            //cout << reqCurPktNo_ << "**********************"<<endl;
            lastFrmNo_ = reqCurPktNo_;
            switchToState(StateFetching);
            requestNextPkt();
        }
        else
        {
            LOG(WARNING) << setw(20) << setfill(' ') << std::right << getDescription()
                         << " meta packet error: "
                         << data->getName() << endl;
            requestMeta();
        }
    }
        break;

    case StateBootstrap:
    case StateFetching:
    {
        int p = Namespacer::findComponent(data->getName(),
                                          NameComponents::NameComponentStreamMetainfo );
        // rcv meta pkt
        if( -1 != p )
        {
            VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                           << " discard antiquated meta pkt" << endl;
        }
        //lastFrmNo_ = MtNdnUtils::frameNumber(data->getName().get(-1));
        lastFrmNo_ = MtNdnUtils::frameNumber(data->getName().get(NameComponents::updateSegNo));
        //int ttl = MtNdnUtils::frameNumber(data->getName().get(NameComponents::dataTTL));
        int ttl = (int)(data->getContent().buf()[0]);
        unsigned int pktNo;
        Namespacer::getFrameNumber(data->getName(),pktNo);

        if( !window_.dataArrived(pktNo) )
        {
            VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                           << " Obsolete Meta" << endl;
            return;
        }

        if( frameBuffer_->getState() == FrameBuffer::Invalid)
            return;

        LOG(INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                  << "RCV " << data->getName().to_uri() << " c=" << reqCurPktNo_
                  << " l=" << lastFrmNo_
                  << " ttl=" << ttl << std::endl;

        int delay = frameBuffer_->recvData(data);
        cs_queue.recvData(*(data.get()), delay);
        requestNextPkt();
    }
        break;

    default:
    {
        VLOG(LOG_ERROR) << setw(20) << setfill(' ') << std::right << getDescription()
                        << "\tState error" << endl;
    }
        break;
    }//switch
}

void
PipelinerFrame::onTimeout(const ptr_lib::shared_ptr<const Interest> &interest)
{
    VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                   << "Timeout " << interest->getName().to_uri()
                 << " ( Loss Rate = " << statistic->getLostRate() << " )"<< endl;

    PacketNumber pktNo;
    Namespacer::getFrameNumber(interest->getName(), pktNo);
    double missrate = window_.dataMissed(pktNo);
    if( missrate > 0.2 && getState() >= StateFetching )
    {
        VLOG(LOG_WARN) << setw(20) << setfill(' ') << std::right << getDescription()
                       << " Restart... (missRate:" << missrate << ")" << endl;
        switchToState(StateInactive); // call back consumer to stop playout and framebuffer
        start();
    }

    switch(state_)
    {
    case StateInactive:
    {

    }
        break;
    case StateWaitInitial:
    {
        int p = Namespacer::findComponent(interest->getName(),
                                          NameComponents::NameComponentStreamMetainfo );
        if( -1 != p )
        {
            requestMeta();
        }
    }
        break;

    case StateBootstrap:
    case StateFetching:
    {
        if( frameBuffer_->getState() == FrameBuffer::Invalid)
            return;
        if( false == frameBuffer_->dataMissed(interest) )
            return;
        else if( 0 )//re-fetch
        {
            VLOG(LOG_TRACE) << setw(20) << setfill(' ') << std::right << getDescription()
                            << "RE-Express " << interest->getName().to_uri() << endl;
            //express(*(interest.get()));
        }

        int reqs = frameBuffer_->getActiveSlots(), ready = frameBuffer_->getReadySlots();
        if( (reqs - ready > 30 || reqs - ready <= 0) && getState() >= StateFetching )
        {
            VLOG(LOG_WARN) << setw(20) << setfill(' ') << std::right << getDescription()
                           << " Restart..." << endl;
            switchToState(StateInactive); // call back consumer to stop playout and framebuffer
            start();
        }

    }
        break;

    default:
    {
        VLOG(LOG_ERROR) << setw(20) << setfill(' ') << std::right << getDescription()
                        << " State error" << endl;
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
    VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                   << "Request Meta: "
                   << metaInterest.getName().toUri() << endl;
}

void
PipelinerFrame::requestNextPkt()
{
    if( reqCurPktNo_ > lastFrmNo_ )
    {
        //usleep(30*1000);
        VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                       << "Forecast Next Frame cn=" << reqCurPktNo_ << " ln=" << lastFrmNo_ << endl;
        scheduleJob(40*1000, [this]()->bool{
            bool res = requestFrame(reqCurPktNo_++);
            if( reqCurPktNo_ > lastFrmNo_ )
            {
                VLOG(LOG_WARN) << setw(20) << setfill(' ') << std::right << getDescription()
                  s          << "Request Forecast Interest cn="
                            << reqCurPktNo_ << " ln=" << lastFrmNo_ << endl;
                return getState() > StateWaitInitial;
            }
            return (!res); // if not requeste, do it again
        }, 1000);
        //scheduleJob(30*1000, bind(&PipelinerFrame::requestFrame, this, reqCurPktNo_));
        //++reqCurPktNo_;
        //requestFrame(reqCurPktNo_++);
    }
    else
    {
        while( reqCurPktNo_ <= lastFrmNo_ )
        {
            requestFrame(reqCurPktNo_++);
        }
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
        VLOG(LOG_WARN) << setw(20) << setfill(' ') << std::right << getDescription()
                       << " Window is exceeded " << window_.getCurrentWindowSize() << endl;
        return false;
    }

    ptr_lib::shared_ptr<Interest> interest = getDefaultInterest( frameNo );
    VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << getDescription()
                    << "REQ " << interest->getName().toUri() << endl;

    if( !frameBuffer_->interestIssued(*interest.get()))
    {
        VLOG(LOG_ERROR) << setw(20) << setfill(' ') << std::right << getDescription()
                        << "RE-REQ Interest " << interest->getName().toUri() << std::endl;
        return false;
    }

    express(*interest.get());
    return true;
}
