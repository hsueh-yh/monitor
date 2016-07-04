#include "consumer.h"
#include <unistd.h>


static boost::asio::io_service libIoService;

Consumer::Consumer (const char* uri) :
    callbackCount_ ( 0 ),
    status(STOPED),
    name(new Name(uri))
{
	/*
	pf = fopen ( "consumer.264", "wb+" );
	if ( pf == NULL )
	{
		cout << "open consumer.264 error" << endl;
		return;
	}
	*/
}

Consumer::~Consumer ()
{
	//while ( 0 != pthread_mutex_destroy ( &recv_buf_mutex ));
	//fclose ( pf );
}


void Consumer::init()
{
    std::cout << "new consumer " << endl;
    NdnRtcUtils::setIoService(libIoService);

    //NdnRtcUtils::performOnBackgroundThread([=]()->void{
            NdnRtcUtils::createLibFace();
    //});
    NdnRtcUtils::startBackgroundThread();

    // Counter holds data used by the callbacks.
    //Consumer consumer(NdnRtcUtils::getLibFace()->getFaceWrapper());
    faceWrapper_ = NdnRtcUtils::getLibFace()->getFaceWrapper();

    frameBuffer_.reset(new FrameBuffer());
    //frameBuffer_->init();
    pipeliner_.reset(new Pipeliner(frameBuffer_, faceWrapper_));

    std::cout << "init consumer " << endl;
    status = READY;
}


void Consumer::start()
{
    if(status != READY)
        init();

    status = STARTED;

	std::cout << "start consumer " << endl;

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


void Consumer::stop()
{

}

