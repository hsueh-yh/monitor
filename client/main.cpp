#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <glogger.h>


std::string createLogDir( const char *root )
{
    fstream rootfile;
    rootfile.open(root, ios::in);
    if(!rootfile)
    {
        std::cout << "Create log dir " << root << std::endl;
        int isCreate = mkdir(root, S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
        if( 0 > isCreate )
        {
            std::cout << "Create log dir failed: " << root << std::endl;
        }
    }
    else
        rootfile.close();

    // get local date
    time_t timestamp = time(NULL);
    tm *date= localtime(&timestamp);

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
	int num = 1;
    string transType; // frame or stream
    for( int i = 1; i < argc; ++i )
    {
        if( strcmp(argv[i], "-n" ) == 0 )
                num = argv[i+1][0] - '0';
        if( strcmp(argv[i], "-t") == 0 )
            transType = argv[i+1];
    }
    QApplication a(argc, argv);

    std::string logfile = createLogDir("./logs");
    GLogger glog( argv[0], logfile.c_str() );
    std::cout << "Log to path: " << logfile << std::endl;

    std::string host = "10.103.246.164";
    unsigned int port = 6363;
    if( argc > 1 )
        host = argv[1];
    if( argc > 2 )
        port = stoi(argv[2]);
    MainWindow w;
    w.setTransType("byFrame");
    w.setHost(host);
    w.setPort(port);
    w.show();

    return a.exec();
}
