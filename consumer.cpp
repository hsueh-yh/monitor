#include "consumer.h"
#include <unistd.h>


//static boost::asio::io_service libIoService;

Consumer::Consumer (const char* uri, boost::shared_ptr<FaceWrapper> faceWrapper) :
    callbackCount_ ( 0 ),
    status_(STOPED),
    name(new Name(uri)),
    faceWrapper_(faceWrapper)
{}


Consumer::~Consumer ()
{
    cout << "Consumer dtor" << endl;
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

    frameBuffer_.reset(new FrameBuffer());
    //frameBuffer_->init();

    pipeliner_.reset(new Pipeliner());
    pipeliner_->init(frameBuffer_,faceWrapper_);

    player_.reset( new Player() );
    player_->init(frameBuffer_);

    status_ = READY;
    //cout << "Consumer init" << endl;
}


void
Consumer::start()
{
    cout << "Consumer start" << endl;
    if(status_ != READY)
    {
        //cout << "Consumer start init" << endl;
        init();
    }

    status_ = STARTED;

    //cout <<"<Pipeliner> count:"<< pipeliner_.use_count() << endl;
    pipeliner_->startFeching();

    /*
	try {
            char tmp[20]="/vide1/01";

            int i = 0;

//            cout << "Consumer: " << (int)getpid() << "-"
//                 << std::this_thread::get_id() << " ";
            //while(i <= 202)
            while (status)
            {

				//Name name("/video/");

				tmp[8]=i+'0';
                name->set(tmp);

				time_t rawtime;
				time(&rawtime);
                name->appendTimestamp(rawtime);

                cout << "Express Interest: " << ++i << " " << name->toUri() << endl;
				// Use bind to pass the counter object to the callbacks.
				faceWrapper_->expressInterest(
                        *name,
						bind(&Pipeliner::onData, pipeliner_.get(), _1, _2),
						bind(&Pipeliner::onTimeout, pipeliner_.get(), _1));

                //if (i>=202)break;
                usleep(50000);

			}


			//ioservice thread is running
            //while(1);
			//sleep(5);

		}
		catch (std::exception& e) {
            cout << endl << "Consumer exception: " << e.what() << endl;
		}
        */
}


void
Consumer::stop()
{
    frameBuffer_.reset();
    pipeliner_.reset();
    player_.reset();

    pipeliner_->stop();
    pipeliner_.reset();

    if( faceWrapper_.use_count() <= 1 )
        faceWrapper_.reset();
}

