#include "mtndn-library.h"
#include "mtndn-manager.h"


IMMNdnLibrary& MMNdnLibrary::getSharedInstance()
{
    return MtNdnManager::getSharedInstance();
}
