#include "name-components.h"
#include "utils.h"


const std::string NameComponents::NameComponentGlobal = "com";
const std::string NameComponents::NameComponentApp = "monitor";
const std::string NameComponents::NameComponentLocation = "location";
const std::string NameComponents::NameComponentDevice = "dev";
const std::string NameComponents::NameComponentStreams = "streams";
const std::string NameComponents::NameComponentStreamFrame = "frame";
const std::string NameComponents::NameComponentFrameSegment = "segment";
//const std::string NameComponents::KeyComponent;
//const std::string NameComponents::CertificateComponent;


std::string
NameComponents::getUserPrefix(const std::string& username,
              const std::string& prefix)
{
    return buildPath(hub[0] != '/',
            &hub,
            &NameComponents::NameComponentApp,
            &NameComponents::NameComponentUser,
            &producerId,
            NULL);
}
