#ifndef MTNDNMANAGER_H
#define MTNDNMANAGER_H

#include "mtndn-library.h"


class MtNdnManager : public IMtNdnLibrary
{
public:
    static MtNdnManager &getSharedInstance();

    void
    setObserver(IMtNdnLibraryObserver *observer);

    std::string
    addRemoteStream(std::string &remoteStreamPrefix,
                    const std::string &threadName,
                    const MediaStreamParams &params,
                    const GeneralParams &generalParams,
                    const GeneralConsumerParams &consumerParams,
                    IExternalRenderer *const renderer);

    std::string
    removeRemoteStream(const std::string &streamPrefix);

    int
    setStreamObserver(const std::string &streamPrefix,
                      IConsumerObserver *const observer);

    int
    removeStreamObserver(const std::string &streamPrefix);

    std::string
    getStreamThread(const std::string &streamPrefix);

    int
    switchThread(const std::string &streamPrefix,
                 const std::string &threadName);

    ~MtNdnManager();
private:
    bool initialized_, failed_;

    MtNdnManager();
    MtNdnManager(MtNdnManager const&) = delete;
    void operator=(MtNdnManager const&) = delete;

    int notifyObserverWithError(const char *format, ...) const;
    int notifyObserverWithState(const char *stateName,
                                const char *format, ...) const;
    void notifyObserver(const char *state, const char *args) const;
};

#endif // MTNDNMANAGER_H
