#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "controler.h"
#include "logger.hpp"


std::string createLogDir( const char* root )
{
    // get local date
    time_t timestamp = time(NULL);
    tm* date= localtime(&timestamp);

    stringstream logDir;
    logDir << date->tm_year + 1900;
    logDir << "-";
    logDir << date->tm_mon + 1;
    logDir << "-";
    logDir << date->tm_mday;
    logDir << "_";
    logDir << date->tm_hour;
    logDir << ":";
    logDir << date->tm_min;
    logDir << ":";
    logDir << date->tm_sec;

    std::string logRoot("./logs/");
    std::string logfile(logRoot);
    logfile.append(logDir.str());

    // create log dir
    int isCreate = mkdir(logfile.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    if( 0 > isCreate )
    {
        std::cout << "Create path failed: " << logfile << std::endl;
    }

    return logfile;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    std::string logfile = createLogDir("./logs");
    GLogger glog( argv[0], logfile.c_str() );
    std::cout << "Log to path: " << logfile << std::endl;


    MainWindow w;
    w.show();

    return a.exec();
}
