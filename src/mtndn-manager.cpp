

//
//  mtndn-manager.cpp
//  mtndn
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
#include "mtndn-namespace.h"
#include "video-consumer.h"
#include "frame-data.h"
#include "mtndn-utils.h"
#include "include/mtndn-defines.h"
#include "simple-log.h"

using namespace ndnlog::new_api;
//using namespace boost;

typedef std::map<std::string, ptr_lib::shared_ptr<Consumer>> ConsumerStreamMap;

static ConsumerStreamMap ActiveStreamConsumers;
static boost::asio::io_service libIoService;
static boost::asio::steady_timer recoveryCheckTimer(libIoService);

void init();
void reset();
void cleanup();

//******************************************************************************
class NdnRtcLibraryInternalObserver:public IMtNdnComponentCallback,
                                    public IMtNdnLibraryObserver
{
public:
    NdnRtcLibraryInternalObserver():libObserver_(NULL){}
    ~NdnRtcLibraryInternalObserver(){}

    void
    errorWithCode(int errCode, const char* message)
    {
        if (libObserver_)
            libObserver_->onErrorOccurred(errCode, message);
    }

    void
    setLibraryObserver(IMtNdnLibraryObserver* libObserver)
    {
        libObserver_ = libObserver;
    }

    // INdnRtcLibraryObserver
    void onStateChanged(const char *state, const char *args)
    {
        if (libObserver_)
            libObserver_->onStateChanged(state, args);
    }

    void onErrorOccurred(int errorCode, const char *errorMessage)
    {
        if (libObserver_)
            libObserver_->onErrorOccurred(errorCode, errorMessage);
    }

    // INdnRtcObjectObserver
    void onErrorOccurred(const char *errorMessage)
    {
        if (libObserver_)
            libObserver_->onErrorOccurred(-1, errorMessage);
    }

    // INdnRtcComponentCallback
    void onError(const char *errorMessage,
                 const int errCode)
    {
        if (libObserver_)
            libObserver_->onErrorOccurred(errCode, errorMessage);
    }

private:
    IMtNdnLibraryObserver* libObserver_;
};

static NdnRtcLibraryInternalObserver LibraryInternalObserver;
//******************************************************************************

//********************************************************************************
//#pragma mark module loading
__attribute__((constructor))
static void initializer(int argc, char* *argv, char* *envp) {
    static int initialized = 0;
    if (!initialized)
    {
        initialized = 1;
        MtNdnUtils::setIoService(libIoService);

    }
}

__attribute__((destructor))
static void destructor(){
}

//******************************************************************************
//#pragma mark - construction/destruction
MtNdnManager::MtNdnManager()
{
    init();
    initialized_ = true;
    failed_ = false;
}

MtNdnManager::~MtNdnManager()
{
    cleanup();
    initialized_ = false;
}

//******************************************************************************
#pragma mark - public
MtNdnManager &MtNdnManager::getSharedInstance()
{
    static MtNdnManager mtndnManager;
    return mtndnManager;
}


void MtNdnManager::setObserver(IMtNdnLibraryObserver *observer)
{
    LOG(INFO) << "Set library observer " << observer << std::endl;

    LibraryInternalObserver.setLibraryObserver(observer);
}


//******************************************************************************
static int id = 0;
std::string
MtNdnManager::addRemoteStream(std::string &remoteStreamPrefix,
                              const std::string &threadName,
                              const MediaStreamParams &params,
                              const GeneralParams &generalParams,
                              const GeneralConsumerParams &consumerParams,
                              IExternalRenderer *const renderer)
{
    VLOG(LOG_INFO) << "MMNdnManager::addRemoteStream " << remoteStreamPrefix << std::endl;
    MtNdnUtils::performOnBackgroundThread([=, &remoteStreamPrefix]()->void{
        //Logger::getLogger(INFO).flush();
        //INFO = NdnUtils::getFullLogPath(generalParams, generalParams.logFile_);
        int threadNum = MtNdnUtils::addBackgroundThread();
        VLOG(LOG_INFO) << "bkg Thread " << threadNum << std::endl;
        MtNdnUtils::createLibFace(generalParams);

        ptr_lib::shared_ptr<Consumer> remoteStreamConsumer;
        ConsumerStreamMap::iterator it = ActiveStreamConsumers.find(remoteStreamPrefix);

        if (it != ActiveStreamConsumers.end() && it->second->getIsConsuming())
        {
            VLOG(LOG_INFO) << "Stream was already added" << std::endl;

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
            settings.faceProcessor_ = MtNdnUtils::getLibFace();

            remoteStreamConsumer->registerCallback(&LibraryInternalObserver);

            if (RESULT_FAIL(remoteStreamConsumer->init(settings, threadName)))
            {
                VLOG(LOG_INFO) << "Failed to initialize fetching from stream" << std::endl;
                remoteStreamPrefix = "";
            }
            else
            {
                /*
                std::string username = Namespacer::getUserName(remoteSessionPrefix);
                std::string logFile = MtNdnUtils::getFullLogPath(generalParams,
                                              MtNdnUtils::formatString("consumer-%s.log",
                                                                    params.streamName_.c_str()));
                */
                std::string logFile = MtNdnUtils::getFullLogPath(generalParams,
                                              MtNdnUtils::formatString("consumer-%d.log", id++));
                //std::string logFile = MtNdnUtils::getFullLogPath(generalParams,
                //                              MtNdnUtils::formatString("consumer-%d.log",id++));
                LOG(INFO) << "logpath " << logFile << std::endl;
                remoteStreamConsumer->setLogger(new Logger(generalParams.loggingLevel_,
                                                           logFile));

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
MtNdnManager::removeRemoteStream(const std::string &streamPrefix)
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
            MtNdnUtils::performOnBackgroundThread([&]()->void{
                ActiveStreamConsumers.erase(it);
            });
        }

        LOG(INFO) << "Stream removed successfully" << std::endl;
    }
    LOG(INFO) << "Remove Remote Stream SUCCESS " << streamPrefix
              << std::endl;

    return logFileName;
}

int
MtNdnManager::setStreamObserver(const std::string &streamPrefix,
                                 IConsumerObserver *const observer)
{
    int res = RESULT_ERR;

    MtNdnUtils::performOnBackgroundThread([=, &res]()->void{
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
MtNdnManager::removeStreamObserver(const std::string &streamPrefix)
{
    int res = RESULT_ERR;

    MtNdnUtils::performOnBackgroundThread([=, &res]()->void{
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
MtNdnManager::getStreamThread(const std::string &streamPrefix)
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
MtNdnManager::switchThread(const std::string &streamPrefix,
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
int MtNdnManager::notifyObserverWithError(const char *format, ...) const
{
    va_list args;

    static char emsg[256];

    va_start(args, format);
    vsprintf(emsg, format, args);
    va_end(args);

    notifyObserver("error", emsg);

    return RESULT_ERR;
}

int MtNdnManager::notifyObserverWithState(const char *stateName, const char *format, ...) const
{
    va_list args;

    static char msg[256];

    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);

    notifyObserver(stateName, msg);

    return RESULT_OK;
}

void MtNdnManager::notifyObserver(const char *state, const char *args) const
{
    LibraryInternalObserver.onStateChanged(state, args);
}

//******************************************************************************
void init()
{
    Logger::initAsyncLogging();
    //GLogger log("MtNdnLibrary","/home/xyh/workspace/MTNDN/logs");
    LOG(INFO) << "MTNDN initializing" << std::endl;
    reset();

    //reset();
}

void reset()
{
    MtNdnUtils::startBackgroundThread();
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

    MtNdnUtils::destroyLibFace();

    MtNdnUtils::stopBackgroundThread();
    LOG(INFO) << "Bye" << std::endl;
    //Logger::releaseAsyncLogging();
}

