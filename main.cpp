#include "mainwindow.h"
#include <QApplication>

#include "controler.h"
#include "logger.hpp"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    GLogger glog(argv[0],"./logs");

    return a.exec();
}
