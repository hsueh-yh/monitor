#include "mtndn-library.h"
#include "mtndn-manager.h"


IMtNdnLibrary& MtNdnLibrary::getSharedInstance()
{
    return MtNdnManager::getSharedInstance();
}
