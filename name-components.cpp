#include "name-components.h"
#include "namespacer.h"
#include "utils.h"


//const std::string NameComponents::NameComponentGlobal = "com";
const std::string NameComponents::NameComponentApp = "monitor";
const std::string NameComponents::NameComponentStreamFrameVideo = "video";
const std::string NameComponents::NameComponentStreamFrameAudio = "audio";
//const std::string NameComponents::KeyComponent;
//const std::string NameComponents::CertificateComponent;

std::string
    getLocationPrefix(const std::string& location,
                  const std::string& prefix)
{

}

std::string
NameComponents::getStreamPrefix(const std::string& username,
              const std::string& prefix)
{
    return *Namespacer::buildPath(hub[0] != '/',
            &hub,
            &NameComponents::NameComponentApp,
            &NameComponents::NameComponentUser,
            &producerId,
            NULL);
}
