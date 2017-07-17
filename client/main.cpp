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
#include <params.h>


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


int interpretParam( int argc, char *argv[], GeneralParams &param )
{
    int streamNum = 1;
    for( int i = 1; i < argc; ++i )
    {
        // stream number
        if( strcmp(argv[i], "-n" ) == 0 )
                streamNum = argv[i+1][0] - '0';
        // stream type
        else if( strcmp(argv[i], "-t") == 0 )
            param.transType_.copy(argv[i+1], strlen(argv[i+1]), 0);
        // next hop host IP
        else if( strcmp(argv[i], "-h") == 0 )
            param.host_.copy(argv[i+1], strlen(argv[i+1]), 0);
        // next hop host port
        else if( strcmp(argv[i], "-h") == 0 )
            param.portNum_ = atoi(argv[i+1]);
        // help
        else if( strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help")  == 0 )
        {
            cout << "\t-k\tvalue  \tdesc [default]" << endl
                 << "\t-n\t[num]  \tnumber of streams you want to add. [1]" << endl
                 << "\t-s\t[str]  \tstream name. ['/com/monitor/location1/stream0/video']" << endl
                 << "\t-t\t[str]  \tstream type:'byFrame'or'byStream'. ['byFrame']" << endl
                 << "\t-h\t[str]  \thost of a NFD. ['10.103.242.127']" << endl
                 << "\t-p\t[num]  \tport in the host. [6363]" << endl
                 << "\t--help\t[] \tthis message" << endl;
            return 0;
        }
    }

    return streamNum;
}


int main(int argc, char *argv[])
{
    GeneralParams param;
    int streamNum = interpretParam( argc, argv, param );


    QApplication a(argc, argv);

    std::string logfile = createLogDir("./logs");
    GLogger glog( argv[0], logfile.c_str() );

    MainWindow w;

    w.setconfigpath("default.conf");
    w.show();

    return a.exec();
}
