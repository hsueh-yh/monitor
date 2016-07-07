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

    bool bFit;


private slots:

    //  on button
    void on_start_btn_clicked();
    void on_stop_btn_clicked();
    void on_add_btn_clicked();
    //  on menu
//    void on_actionAdd_Face_clicked();
//    void on_actionAdd_Stream_clicked();

    void addStream(QString stream);

private:

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
};

#endif // MAINWINDOW_H
