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

#include "consumer.h"
#include "player.h"


class Controler
{
public:
    Controler():
        consumerNum_(0)
    {}

    ~Controler()
    {}

    int addConsumer();
    int startConsumer(int id);
    int stopConsumer(int id);

    bool addFace( int port );


//private:
    int consumerNumber_;
    int faceNumber_;
    map<int,Consumer*> consumersMap_;
    vector<Consumer*> consumersVec_;
//    Consumer* consumer;
//    Player* player_;

};

#endif // CONTROLER_H
