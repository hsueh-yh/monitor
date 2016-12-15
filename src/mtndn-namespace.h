#ifndef NAMESPACER_H
#define NAMESPACER_H

// Name:  .../monitor/<location>/<streams>/video(audio)/<frameNumber>/<segmentNumber>/<PrefixMetaInfo>

#include <string>
#include <ndn-cpp/name.hpp>

#include "name-components.h"
#include "frame-data.h"
#include "mtndn-common.h"
#include "mtndn-utils.h"

class Namespacer {
public:

    static ptr_lib::shared_ptr<std::string>
        buildPath(bool precede, const std::string *component1, ...);

    static int findComponent(const Name &prefix,
                             const std::string &component);

    //*****************************************************
    //  get prefix with string
    //*****************************************************

    static ptr_lib::shared_ptr<std::string>
        getLocationPrefix(const std::string &prefix,
                          const std::string &location);

    static ptr_lib::shared_ptr<std::string>
        getStreamPath(const std::string &prefix,
                      const std::string &location,
                      const std::string &streamName);

    static ptr_lib::shared_ptr<std::string>
        getStreamVideoPath(const std::string &prefix,
                           const std::string &location,
                           const std::string &streamName);

    static ptr_lib::shared_ptr<std::string>
        getStreamAudioPath(const std::string &prefix,
                           const std::string &location,
                           const std::string &streamName);

    static void getFramePrefix(const Name &prefix, Name &subPrefix);

    static void getSegmentPrefix(const Name &prefix, Name &subPrefix);

    static void getStreamVideoPrefix(const Name &prefix, Name &subPrefix);

    //*****************************************************
    //  get name component with corresponding struct
    //*****************************************************

    static void getSegmentationNumbers(const ndn::Name &prefix,
                                       PacketNumber &packetNumber,
                                       SegmentNumber &segmentNumber);

    static void getPacketNumber(const ndn::Name &prefix,
                                       PacketNumber &packetNumber);

    static void getFrameNumber(const ndn::Name &prefix,
                                       FrameNumber &packetNumber);

    static void getSegmentNumber(const ndn::Name &prefix,
                                       SegmentNumber &segmentNumber);

    static void getPrefixMetaInfo(const ndn::Name &prefix,
                                  PrefixMetaInfo &prefixMetaInfo);

    static void getMetaNumber(const ndn::Name &prefix,
                              PacketNumber &packetNumber);
};

#endif // NAMESPACER_H
