#include "controler.h"

static boost::asio::io_service libIoService;


Controler::Controler():
    consumerNumber_(0)
{
    newFace("10.103.240.101", 6363);
}


int
Controler::addConsumer( std::string prefix, int face )
{
    int consumerId = ++consumerNumber_;

    Consumer *consumer = new Consumer(prefix.c_str(), FaceWrapper_);
    consumersMap_.insert(pair<int,Consumer*>(consumerId,consumer));

    std::cout << "Add Consumer " << consumerId << std::endl;

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
    std::cout << "Start Consumer " << consumerId << std::endl;
    Consumer *consumer = getConsumer(consumerId);
    if( consumer == NULL )
    {
        std::cout << "Can not start Consumer " << consumerId << std::endl;
        return -1;
    }

    consumer->init();

    std::thread *consumerThread =
            new std::thread(bind(&Consumer::start,consumer));
//            new std::thread([&]
//                {
//                    consumer->start();
//                });

    consumerThread->detach();
    return 1;
}


int Controler::stopConsumer(int consumerId)
{
    std::cout << endl << "Stop Consumer " << consumerId << std::endl;
    Consumer *consumer = getConsumer(consumerId);
    if( consumer == NULL )
        return -1;

    consumer->stop();
    return 1;
}


void
Controler::newFace(const std::string host, const int port)
{
    NdnRtcUtils::setIoService(libIoService);

    //NdnRtcUtils::performOnBackgroundThread([=]()->void{
            NdnRtcUtils::createLibFace(host, port);
    //});
    NdnRtcUtils::startBackgroundThread();

    // Counter holds data used by the callbacks.
    //Consumer consumer(NdnRtcUtils::getLibFace()->getFaceWrapper());
    FaceWrapper_ = NdnRtcUtils::getLibFace()->getFaceWrapper();
}
