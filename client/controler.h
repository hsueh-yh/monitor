#ifndef CONTROLER_H
#define CONTROLER_H

#include <cstdlib>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <thread>
#include <map>
#include <vector>

#include <QLabel>

#include "src/consumer.h"
#include "src/player.h"


class Controler
{
public:
    Controler();

    ~Controler()
    {}

    int addStream(std::string prefix);

    int addConsumer(std::string prefix);

    int startConsumer(int consumerId);

    int stopConsumer(int consumerId);

    Consumer *getConsumer(const int consumerId );

    void addFace(const string host, const int port);

    void
    lock()  { syncMutex_.lock(); }

    void
    unlock() { syncMutex_.unlock(); }


//private:
    int consumerNumber_,maxId_;
    int faceNumber_;
    map<int,Consumer*> consumersMap_;
    map<int,pthread_t> playerId_;

    set<std::string> hostSet_;

    ptr_lib::shared_ptr<FaceWrapper> FaceWrapper_;

    std::mutex syncMutex_;

};

#endif // CONTROLER_H
