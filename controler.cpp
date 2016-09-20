#include "controler.h"
#include <iostream>
#include "logger.hpp"

#define HOST "localhost"
//#define HOST "10.103.242.213"
//#define HOST "10.103.243.176"
#define PORT 6363

static boost::asio::io_service libIoService;


Controler::Controler():
    consumerNumber_(0),
    maxId_(0)
{
    consumersMap_.clear();
    consumersVec_.clear();
    hostSet_.clear();
    addFace(HOST,PORT);
}


int
Controler::addStream( std::string prefix)
{
#ifdef __SHOW_CONSOLE_
    std::cout << "[Controler] Add Stream: " << prefix <<endl;
#endif
    int consumerId = addConsumer( prefix );
    startConsumer(consumerId);
    return consumerId;
}


int
Controler::addConsumer( std::string prefix )
{
    int consumerId = ++maxId_;
    //int consumerId = ++consumerIdx_;
    consumerNumber_++;

    Consumer *consumer = new Consumer(prefix.c_str(), FaceWrapper_);
    consumersMap_.insert(pair<int,Consumer*>(consumerId,consumer));

    LOG(INFO) << "[Controler] Adding consumer: \"" << prefix << "\""
                 << "ID: "<< consumerId;
    return consumerId;
}


Consumer*
Controler::getConsumer( const int consumerId )
{
    map<int,Consumer*>::iterator iter;

    iter = consumersMap_.find(consumerId);

    if( iter == consumersMap_.end() )
    {
        return NULL;
    }

    return iter->second;
}


int
Controler::startConsumer( int consumerId )
{
    LOG(INFO) << "[Controler] Starting consumer ID: " << consumerId;
    Consumer *consumer = getConsumer(consumerId);
    if( consumer == NULL )
    {
        std::cout << "Can not start Consumer " << consumerId << std::endl;
        LOG(WARNING) << "[Controler] Fail to finding consumer when Starting consumer ID: " << consumerId;
        return -1;
    }

    consumer->init();

    std::thread *consumerThread =
            new std::thread(bind(&Consumer::start,consumer));

    consumerThread->detach();

    return 1;
}


int
Controler::stopConsumer(int consumerId)
{
    if(consumerId<=0)
    {
        cout << "stop " << consumerId << "fail" << endl;
        return -1;
    }
    lock();
#ifdef __SHOW_CONSOLE_
    std::cout << endl << "[Controler] Stop Consumer " << consumerId << std::endl;
#endif
    if(consumerNumber_ > 0)
        consumerNumber_--;
    else
    {
        cout << "stop " << consumerId << "fail (consumerNumber_ <= 0)" << endl;
        return -1;
    }
    Consumer *consumer = getConsumer(consumerId);

    if( consumer == NULL )
    {
        cout << "stop " << consumerId << "fail (not found consumer)" << endl;
        return -1;
    }

    map<int,Consumer*>::iterator iter;

    iter = consumersMap_.find(consumerId);

    if( iter != consumersMap_.end() )
    {
        consumersMap_.erase(iter);
    }


    consumer->stop();
    //delete consumer;
    unlock();
    return 1;
}


void
Controler::addFace(const std::string host, const int port)
{
    /*
    // if the uri did not specify a port
    if(std::string::npos == uri.find(":"))
        uri.append(":6363");

    //check ip format (not useful)


    set<std::string>::iterator iter;
    iter = hostSet_.find(uri);
    // the face already exists
    if(iter != hostSet_.end())
        return;

    hostSet_.insert(uri);
    */

#ifdef __SHOW_CONSOLE_
    std::cout << "Create Face: " << host<< ":"<<port << endl;
#endif

    NdnRtcUtils::setIoService(libIoService);

    //NdnRtcUtils::performOnBackgroundThread([=]()->void{
            NdnRtcUtils::createLibFace(host, port);
    //});
    NdnRtcUtils::startBackgroundThread();

    // Counter holds data used by the callbacks.
    //Consumer consumer(NdnRtcUtils::getLibFace()->getFaceWrapper());
    FaceWrapper_ = NdnRtcUtils::getLibFace()->getFaceWrapper();
}
