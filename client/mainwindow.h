#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <map>
#include <iostream>

#include <QtGui>
#include <QLayout>

#include "addstreamdialog.h"

#include "simulator.h"
#include "myTimer.h"
#include "renderer.h"

#include "config.h"
#include <mtndn-library.h>

using namespace std;


namespace Ui {
class MainWindow;
}

class MainWindow :  public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0 );
    ~MainWindow();


    void
    setconfigpath(std::string file)
    {
        configFile = file;
    }


private:

    Ui::MainWindow *ui;
    AddStreamDialog *addStreamDialog;

    QPainter *painters;
    QPixmap pixmap;
    vector<QImage*> imageVec;

    //Simulator
    Simulator *simulator;
    MyTimer *mytimer_;

    IMtNdnLibrary *manager_;
    //vector<RendererInternal*> renderer_;
    typedef map<std::string, RendererInternal*> mapRenderer;
    mapRenderer map_str_renderer_;

    unsigned int label_width,
                 label_height;

    int space_;

    /////////////////////////////
//    struct timeval t_1, t_2;
//    long lt_1,lt_2;

    //mtlib params
    vector<CParams*> vec_params_;
    std::string configFile;


    CParams*
    loadDefaultParams();


    void
    closeEvent(QCloseEvent *event);

    int
    refreshRenderer();

    bool
    calLabelParam(int idx, int &x, int &y, int &width, int &height);

    void
    stop_all()
    {
        mapRenderer::iterator it = map_str_renderer_.begin();
        while( it != map_str_renderer_.end() )
        {
            manager_->removeRemoteStream(it->first);
        }
        map_str_renderer_.clear();
    }

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

};

#endif // MAINWINDOW_H
