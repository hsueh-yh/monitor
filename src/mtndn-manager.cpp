

//
//  ndnrtc-manager.cpp
//  ndnrtc
//
//  Copyright 2015 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory>
#include <signal.h>
#include <boost/thread/mutex.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "mtndn-manager.h"
#include "namespacer.h"
#include "video-consumer.h"
#include "frame-data.h"
#include "utils.h"
#include "defines.h"

//using namespace boost;

typedef std::map<std::string, ptr_lib::shared_ptr<Consumer>> ConsumerStreamMap;

static ConsumerStreamMap ActiveStreamConsumers;
static boost::asio::io_service libIoService;
static boost::asio::steady_timer recoveryCheckTimer(libIoService);

void init();
void cleanup();


//********************************************************************************
//#pragma mark module loading
__attribute__((constructor))
static void initializer(int argc, char* *argv, char* *envp) {
    static int initialized = 0;
    if (!initialized)
    {
        initialized = 1;
        NdnUtils::setIoService(libIoService);

    }
}

__attribute__((destructor))
static void destructor(){
}


//******************************************************************************
//#pragma mark - construction/destruction
MMNdnManager::MMNdnManager()
{
    init();
    initialized_ = true;
    failed_ = false;
}

MMNdnManager::~MMNdnManager()
{
    cleanup();
    initialized_ = false;
}

//******************************************************************************
#pragma mark - public
MMNdnManager &MMNdnManager::getSharedInstance()
{
    static MMNdnManager mtndnManager;
    return mtndnManager;
}

/*
void MtNdnManager::setObserver(INdnRtcLibraryObserver *observer)
{
    LOG(INFO) << "Set library observer " << observer << std::endl;

    LibraryInternalObserver.setLibraryObserver(observer);
}
*/

//******************************************************************************
std::string
MMNdnManager::addRemoteStream(std::string &remoteStreamPrefix,
                              const std::string &threadName,
                              const MediaStreamParams &params,
                              const GeneralParams &generalParams,
                              const GeneralConsumerParams &consumerParams,
                              IExternalRenderer *const renderer)
{
    LOG(INFO) << "addRemoteStream" << std::endl;
    NdnUtils::performOnBackgroundThread([=, &remoteStreamPrefix]()->void{
        //Logger::getLogger(INFO).flush();
        //INFO = NdnUtils::getFullLogPath(generalParams, generalParams.logFile_);
        NdnUtils::createLibFace(generalParams.host_,generalParams.portNum_);

        ptr_lib::shared_ptr<Consumer> remoteStreamConsumer;
        ConsumerStreamMap::iterator it = ActiveStreamConsumers.find(remoteStreamPrefix);

        if (it != ActiveStreamConsumers.end() && it->second->getIsConsuming())
        {
            LOG(INFO) << "Stream was already added" << std::endl;

            remoteStreamPrefix = "";
        }
        else
        {
            if (params.type_ == MediaStreamParams::MediaStreamTypeAudio)
            {
                //remoteStreamConsumer.reset(new AudioConsumer(generalParams, consumerParams));
            }
            else
                remoteStreamConsumer.reset(new VideoConsumer(generalParams, consumerParams, renderer));

            if (it != ActiveStreamConsumers.end())
                it->second = remoteStreamConsumer;

            ConsumerSettings settings;
            settings.streamPrefix_ = remoteStreamPrefix;
            settings.streamParams_ = params;
            settings.faceProcessor_ = NdnUtils::getLibFace();

            //remoteStreamConsumer->registerCallback(&LibraryInternalObserver);

            if (RESULT_FAIL(remoteStreamConsumer->init(settings, threadName)))
            {
                LOG(INFO) << "Failed to initialize fetching from stream" << std::endl;
                remoteStreamPrefix = "";
            }
            else
            {
                /*
                std::string username = Namespacer::getUserName(remoteSessionPrefix);
                std::string logFile = NdnUtils::getFullLogPath(generalParams,
                                              NdnUtils::toString("consumer-%s-%s.log",
                                                                    username.c_str(),
                                                                    params.streamName_.c_str()));

                remoteStreamConsumer->setLogger(new Logger(generalParams.loggingLevel_,
                                                           logFile));
                */
                if (RESULT_FAIL(remoteStreamConsumer->start()))
                    remoteStreamPrefix = "";
                else
                {
                    if (it == ActiveStreamConsumers.end())
                        ActiveStreamConsumers[remoteStreamConsumer->getPrefix()] = remoteStreamConsumer;
                    remoteStreamPrefix = remoteStreamConsumer->getPrefix();
                }
            }
        }
    });

    return remoteStreamPrefix;
}

std::string
MMNdnManager::removeRemoteStream(const std::string &streamPrefix)
{
    std::string logFileName = "";

    LOG(INFO) << "Removing stream " << streamPrefix << "..." << std::endl;

    ConsumerStreamMap::iterator it = ActiveStreamConsumers.find(streamPrefix);

    if (it == ActiveStreamConsumers.end())
    {
        LOG(INFO) << "Stream was not added previously" << std::endl;
    }
    else
    {
        //logFileName = it->second->getLogger()->getFileName();
        it->second->stop();
        {
            NdnUtils::performOnBackgroundThread([&]()->void{
                ActiveStreamConsumers.erase(it);
            });
        }

        LOG(INFO) << "Stream removed successfully" << std::endl;
    }

    return logFileName;
}

int
MMNdnManager::setStreamObserver(const std::string &streamPrefix,
                                 IConsumerObserver *const observer)
{
    int res = RESULT_ERR;

    NdnUtils::performOnBackgroundThread([=, &res]()->void{
        LOG(INFO) << "Setting stream observer " << observer
        << " for stream " << streamPrefix << "..." << std::endl;

        ConsumerStreamMap::iterator it = ActiveStreamConsumers.find(streamPrefix);

        if (it == ActiveStreamConsumers.end())
        {
            LOG(INFO) << "Stream was not added previously" << std::endl;
        }
        else
        {
            res = RESULT_OK;
            it->second->registerObserver(observer);
            LOG(INFO) << "Added observer successfully" << std::endl;
        }
    });

    return res;
}

int
MMNdnManager::removeStreamObserver(const std::string &streamPrefix)
{
    int res = RESULT_ERR;

    NdnUtils::performOnBackgroundThread([=, &res]()->void{
        LOG(INFO) << "Removing stream observer for prefix " << streamPrefix
        << "..." << std::endl;

        ConsumerStreamMap::iterator it = ActiveStreamConsumers.find(streamPrefix);

        if (it == ActiveStreamConsumers.end())
        {
            LOG(INFO) << "Couldn't find requested stream" << std::endl;
        }
        else
        {
            res = RESULT_OK;
            it->second->unregisterObserver();
            LOG(INFO) << "Stream observer was removed successfully" << std::endl;
        }
    });

    return res;
}

std::string
MMNdnManager::getStreamThread(const std::string &streamPrefix)
{
    /*
    ConsumerStreamMap::iterator it = ActiveStreamConsumers.find(streamPrefix);

    if (it == ActiveStreamConsumers.end())
    {
        return "";
    }

    return it->second->getCurrentThreadName();
    */
    return std::string("");
}

int
MMNdnManager::switchThread(const std::string &streamPrefix,
                            const std::string &threadName)
{
    /*
    int res = RESULT_ERR;

    NdnUtils::performOnBackgroundThread([=, &res]()->void{
        LOG(INFO) << "Switching to thread " << threadName
        << " for stream " << streamPrefix << std::endl;

        ConsumerStreamMap::iterator it = ActiveStreamConsumers.find(streamPrefix);

        if (it == ActiveStreamConsumers.end())
        {
            LOG(INFO) << "Couldn't find requested stream" << std::endl;
        }
        else
        {
            res = RESULT_OK;
            it->second->switchThread(threadName);
            LOG(INFO) << "Thread switched successfully" << std::endl;
        }
    });
*/
    return RESULT_OK;
}


//********************************************************************************
#pragma mark - private
int MMNdnManager::notifyObserverWithError(const char *format, ...) const
{
    va_list args;

    static char emsg[256];

    va_start(args, format);
    vsprintf(emsg, format, args);
    va_end(args);

    notifyObserver("error", emsg);

    return RESULT_ERR;
}

int MMNdnManager::notifyObserverWithState(const char *stateName, const char *format, ...) const
{
    va_list args;

    static char msg[256];

    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);

    notifyObserver(stateName, msg);

    return RESULT_OK;
}

void MMNdnManager::notifyObserver(const char *state, const char *args) const
{
    //LibraryInternalObserver.onStateChanged(state, args);
}

//******************************************************************************
void init()
{
    //GLogger log("MtNdnLibrary","/home/xyh/workspace/MTNDN/logs");
    LOG(INFO) << "MTNDN " << std::endl;

    //reset();
}

void cleanup()
{
    LOG(INFO) << "Stopping active consumers..." << std::endl;
    {
        for (auto consumerIt:ActiveStreamConsumers)
            consumerIt.second->stop();
        ActiveStreamConsumers.clear();
    }
    LOG(INFO) << "Active consumers cleared" << std::endl;

    NdnUtils::destroyLibFace();

    NdnUtils::stopBackgroundThread();
    LOG(INFO) << "Bye" << std::endl;
    //Logger::releaseAsyncLogging();
}

