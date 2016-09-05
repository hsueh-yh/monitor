#ifndef NAMECOMPONENTS_H
#define NAMECOMPONENTS_H

#include <string>

class NameComponents {
public:
    static const std::string NameComponentGlobal;
    static const std::string NameComponentApp;
    static const std::string NameComponentLocation;
    static const std::string NameComponentDevice;
    static const std::string NameComponentStreams;
    static const std::string NameComponentStreamFrame;
    static const std::string NameComponentFrameSegment;
//    static const std::string KeyComponent;
//    static const std::string CertificateComponent;

    static std::string
    getUserPrefix(const std::string& username,
                  const std::string& prefix);

    static std::string
    getStreamPrefix(const std::string& streamName,
                    const std::string& username,
                    const std::string& prefix);

    static std::string
    getThreadPrefix(const std::string& threadName,
                    const std::string& streamName,
                    const std::string& username,
                    const std::string& prefix);
};

#endif // NAMECOMPONENTS_H
