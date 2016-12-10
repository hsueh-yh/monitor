#ifndef LOGGER_H_
#define LOGGER_H_

#include <glog/logging.h>
#include <glog/raw_logging.h>

#include <fstream>
#include <string>


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
