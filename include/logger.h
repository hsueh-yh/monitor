#ifndef LOGGER_H_
#define LOGGER_H_

#include <glog/logging.h>
#include <glog/raw_logging.h>

#include <fstream>
#include <string>

enum LogLevel {
  LOG_FATAL          = -1, // fatal (will be logged unconditionally)
  LOG_NONE           = 0, // no messages
  LOG_ERROR          = 1, // serious error messages
  LOG_WARN           = 2, // warning messages
  LOG_INFO           = 3, // informational messages
  LOG_DEBUG          = 4, // debug messages
  LOG_TRACE          = 5, // trace messages (most verbose)
  LOG_ALL            = 255 // all messages
};

class GLogger
{
public:
    GLogger( const char *program, const char *logdir);
    ~GLogger();

    //将信息输出到单独的文件和 LOG(ERROR)
/*   static void SignalHandle(const char *data, int size)
    {
        std::string dumpPath(logdir_);
        dumpPath.append("/glog_dump.log");
        std::ofstream fs(dumpPath.c_str(),std::ios::app);
        std::string str = std::string(data,size);
        fs<<str;
        fs.close();
        LOG(ERROR)<<str;
    }
*/
    std::string logdir_;
};


#endif // LOGGER_H_
