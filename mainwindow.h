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
    //  on menu
//    void on_actionAdd_Face_clicked();
//    void on_actionAdd_Stream_clicked();

public slots:
    void addStream(string stream_);

//private:
public:
    void simulatorWork(int index, double duration/*, const boost::system::error_code& e*/ );
    void showStream(int id);


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

    boost::asio::steady_timer *timer;

    double *durations;
    int *jobs;
    int counter = 0;

    // pareto parameters
    double  p_alpha = 0.6,
            p_x_min = 5.0,
            p_min = 5.0,
            p_max = 10.0;
    int     p_quantity = 10;

    // zipf parameters
    double  z_alpha = 0.6,
            z_min = 0,
            z_max = 100;
    int     z_quantity = 100;

};

#endif // MAINWINDOW_H
