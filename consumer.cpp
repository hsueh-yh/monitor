#include "consumer.h"
#include <unistd.h>


//static boost::asio::io_service libIoService;

Consumer::Consumer (std::string uri, boost::shared_ptr<FaceWrapper> faceWrapper) :
    callbackCount_ ( 0 ),
    status_(STOPED),
    prefix_(uri),
    //name(new Name(uri)),
    faceWrapper_(faceWrapper)
{}


Consumer::~Consumer ()
{
//    if(player_.use_count() > 0)
//        delete player_.get();
//    if(pipeliner_.use_count() > 0)
//        delete pipeliner_.get();
//    if(frameBuffer_.use_count() > 0)
//        delete frameBuffer_.get();

//    player_.reset();
//    pipeliner_.reset();
//    frameBuffer_.reset();
#ifdef __SHOW_CONSOLE_
    cout << "[Consumer] dtor" << endl;
#endif
}


void
Consumer::init()
{
//    std::cout << "new consumer " << endl;
//    NdnRtcUtils::setIoService(libIoService);

//    //NdnRtcUtils::performOnBackgroundThread([=]()->void{
//            NdnRtcUtils::createLibFace("localhost",6363);
//    //});
//    NdnRtcUtils::startBackgroundThread();

//    // Counter holds data used by the callbacks.
//    //Consumer consumer(NdnRtcUtils::getLibFace()->getFaceWrapper());
//    faceWrapper_ = NdnRtcUtils::getLibFace()->getFaceWrapper();

    //frameBuffer_.reset();
    frameBuffer_.reset(new FrameBuffer());
    //frameBuffer_->init();

    //pipeliner_.reset();
    pipeliner_.reset(new Pipeliner(this->prefix_));
    pipeliner_->init(frameBuffer_,faceWrapper_);

    //player_.reset();
    player_.reset( new Player() );
    player_->init(frameBuffer_);

    status_ = READY;
}


void
Consumer::start()
{
    if(status_ != READY)
    {
        //cout << "Consumer start init" << endl;
        init();
    }

    status_ = STARTED;

    //cout <<"<Pipeliner> count:"<< pipeliner_.use_count() << endl;
    pipeliner_->startFetching();

}


void
Consumer::stop()
{
    status_ = STOPED;
    player_->stop();
    pipeliner_->stop();
    frameBuffer_->stop();
    //faceWrapper_->shutdown();

    /*
    player_->~Player();
    player_.reset();

    pipeliner_->stop();
    pipeliner_->~Pipeliner();
    pipeliner_.reset();

    frameBuffer_->~FrameBuffer();
    */
    //frameBuffer_.reset();

//    if( faceWrapper_.use_count() <= 1 )
//        faceWrapper_.reset();
}

