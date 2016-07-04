#include <cstdlib>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <thread>

#include "consumer.h"
#include "player.h"


using namespace std;



int main1(int argc, char** argv)
{
	std::cout << "start" << endl;
	Consumer *consumer = new Consumer();

	std::cout << "init" << endl;
	consumer->init();
	Player *player = new Player(consumer->frameBuffer_);
//	if(player->init())
//		cout << "Player init" << endl;
//	else
//		cout << "Player init failed" << endl;

	std::thread writeFileThread([&]
		{
			player->writeFile();
		});
	writeFileThread.detach();
	std::cout << "write thread started" << endl;
	consumer->start();

/*	try {
		// The default Face will connect using a Unix socket, or to "localhost".
		//Face face;


		NdnRtcUtils::setIoService(libIoService);

		//NdnRtcUtils::startBackgroundThread();

		//NdnRtcUtils::performOnBackgroundThread([=]()->void{
		        NdnRtcUtils::createLibFace();

		//    });
		        NdnRtcUtils::startBackgroundThread();
		        //std::thread ioservice([&]{ NdnRtcUtils::getIoService().run(); });
		        //ioservice.detach();

		//NdnRtcUtils::getIoService().run();
		//dnRtcUtils::createLibFace();

		// Counter holds data used by the callbacks.
		Consumer consumer(NdnRtcUtils::getLibFace()->getFaceWrapper());

		char tmp[20]="/vide1/01";
		Name name;
		cout << "sending interests..." << endl;
		for (int i = 0; i < 200; i++)
		{
			//Name name("/video/");
			
			tmp[8]=i+'0';
			
			name.set(tmp);
			time_t rawtime;
			time(&rawtime);
			name.appendTimestamp(rawtime);
	
			cout << "Express name " << name.toUri() << endl;
			// Use bind to pass the counter object to the callbacks.
			consumer.faceWrapper_->expressInterest(
					name,
					bind(&Consumer::onData, &consumer, _1, _2),
					bind(&Consumer::onTimeout, &consumer, _1));

			consumer.callbackCount_ = 0;
		}
		//});

		//ioservice thread is running
		while(1);
	}
	catch (std::exception& e) {
		cout << "exception: " << e.what() << endl;
	}
*/
	return 0;
}
