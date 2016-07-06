#include "mainwindow.h"
#include "controler.h"
#include <QApplication>

#define __SHOW_CONSOLE_

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
