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

#include "mtndn-library.h"
#include "renderer.h"

using namespace std;


namespace Ui {
class MainWindow;
}

class MainWindow :  public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

 //   void paintEvent(QPaintEvent *event);
    void setPixmap(const uchar *buf);

    //void startStream(string stream);

    bool onFrameRefresh( const int id, const unsigned char *pframe,
                         const unsigned int width, const unsigned int height );

    void onFrameDelivered( const unsigned char *frame );

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

    void closeEvent(QCloseEvent *event);

    void showStream(int id);

    void refreshLabel(const int id, const unsigned char *pframe,
                      const unsigned int width, const unsigned int height);

    int refreshRenderer();

    void work();

    std::shared_ptr<Consumer>
    getConsumer( int id );

    QLabel *getLabel( int id );

    Ui::MainWindow *ui;
    //Controler *controler;
    //vector<QPainter*> painters;
    Controler *controler;
    QPainter *painters;

    QPixmap pixmap;
    vector<QImage*> imageVec;

    map<int, std::shared_ptr<Consumer> > mapConsumers;
    map<int,QLabel*> mapLabels;

    int activedConsumerNum_;
    int alloctedId_;
    unsigned int label_width,
                 label_height;

    int space_;

    AddStreamDialog *addStreamDialog;


    //Simulator
    Simulator *simulator;
    //QTimer *timer;

    MyTimer *mytimer_;

    IMMNdnLibrary *manager_;

    vector<RendererInternal*> renderer_;

//    struct timeval t_1, t_2;
//    long lt_1,lt_2;

};

#endif // MAINWINDOW_H
