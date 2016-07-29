#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <map>
#include <iostream>

#include <QtGui>
#include <QLayout>

#include "addstreamdialog.h"

#include "controler.h"
//#include "consumer.h"
#include "simulator.h"
#include "myTimer.h"

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

    //void startStream(string stream);

    bool bFit;


private slots:

    //  on button
    void on_start_btn_clicked();
    void on_stop_btn_clicked();
    void on_add_btn_clicked();
    void on_simulate_btn_clicked();
    void on_simulate_waiting(int simuId);
    void on_simulate_fetching(int id);
    //  on menu
//    void on_actionAdd_Face_clicked();
//    void on_actionAdd_Stream_clicked();


public slots:
    int addStream(string stream_);
    int stopStream(int id);

private:
    void showStream(int id);
    void work();

    Ui::MainWindow *ui;
    //Controler* controler;
    //vector<QPainter*> painters;
    Controler *controler;
    QPainter *painters;

    QPixmap pixmap;
    vector<QImage*> imageVec;
    map<int,QLabel*> labelMap;
    int space_;

    AddStreamDialog *addStreamDialog;


    //Simulator
    Simulator *simulator;
    //QTimer *timer;

    MyTimer *mytimer_;

//    struct timeval t_1, t_2;
//    long lt_1,lt_2;

};

#endif // MAINWINDOW_H
