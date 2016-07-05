#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <iostream>

#include <QtGui>

#include "controler.h"
//#include "consumer.h"

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

 //   void paintEvent(QPaintEvent *event);
    void setPixmap(const uchar* buf);

    bool bFit;


private slots:

    //  on button
    void on_start_btn();
    void on_stop_btn();
    //  on menu
//    void on_actionAdd_Face_clicked();
//    void on_actionAdd_Stream_clicked();


private:

    void showTable(int id);

    Ui::MainWindow *ui;
    //Controler* controler;
    //vector<QPainter*> painters;
    Controler *controler;
    QPainter *painters;

    QPixmap pixmap;
    QImage *image;
};

#endif // MAINWINDOW_H
