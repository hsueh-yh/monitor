#ifndef MTNDNLIBRARY_H
#define MTNDNLIBRARY_H

#include "params.h"
//#include "statistics.h"
#include "interfaces.h"


/**
  *This class provides interface to work with NDN-RTC library.
  *It provides calls to allow publish audio/video streams and fetch them.
  *User should create an instance of this class using static methods
  *provided. All further communications with NDN-RTC library should be
  *performed using this instance.
  *No upper boundary exists for the number of simultaneously fetched
  *streams. Library is configured using ParamsStruct structure.
 */
class MMNdnLibrary {
public:
    /**
      *Returns a reference to the singleton instance of MtNdnLirary class
     */
    static IMMNdnLibrary &getSharedInstance();

private:
    MMNdnLibrary();
    MMNdnLibrary(MMNdnLibrary const&) = delete;
    void operator=(MMNdnLibrary const&) = delete;
};


#endif // MTNDNLIBRARY_H
