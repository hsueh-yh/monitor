#include "controler.h"

int Controler::addConsumer()
{
    int consumerId = consumerNum_;
    consumerNumber_++;

    std::cout << "start" << endl;

    Consumer *consumer = new Consumer("/vide1/01");
    consumersVec_.push_back(consumer);
    player_ = new Player();

    //consumers.insert(pair <int, Consumer*> (consumerId,consumer));
    //consumers.push_back(consumer);
//    QLabel *label = new QLabel();
//    labels.insert(pair<int,QLabel*>(consumerId,label));

    //consumer->start();
//    std::thread consumerThread([&]
//        {
//            consumer->start();
//        });
//        consumerThread.detach();

    return consumerId;
}


int Controler::startConsumer(int id)
{
    consumer->init();
    std::thread consumerThread([&]
                    {
                        consumer->start();
                    });
    consumerThread.detach();
    cout << "consumer thread started" << endl;
}

int Controler::stopConsumer(int id)
{

}
