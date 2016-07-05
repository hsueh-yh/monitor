#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    controler(new Controler)
{
    ui->setupUi(this);
   // ui->txlabel->resize(640,480);

    pixmap = QPixmap(640,480);
    bFit = true;
    //ui->txlabel->resize(640,480);

    connect(ui->start_btn,SIGNAL(clicked()),this,SLOT(on_start_btn()));
    connect(ui->stop_btn,SIGNAL(clicked()),this,SLOT(on_stop_btn()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void
MainWindow::showTable( int id )
{
    cout << "Start Player " << id << endl;

    Consumer *consumer = controler->getConsumer(id);

    if(consumer == NULL)
    {
        cout << "Consumer "<< id << " not started" << endl;
        return ;
    }

    unsigned char *tmp = consumer->player_->bmp_frameBuf_;
    image = new QImage(tmp,640,480,QImage::Format_RGB888);

    int i = 0;
    while( ++i  /*<= 202*/ )
    {
        //unsigned char *tmp = controler->getConsumer(id)->player_->bmp_frameBuf_;
        //image = new QImage(tmp,640,480,QImage::Format_RGB888);
        if( tmp == NULL )
        {
            ui->txlabel->setText("helloworld");
        }
        else
        {
            //setPixmap(tmp);
            //image = new QImage(tmp,640,480,QImage::Format_RGB888);
            controler->getConsumer(id)->player_->refresh();
            pixmap = QPixmap::fromImage(*image);
            ui->txlabel->setPixmap(pixmap);
            ui->txlabel->show();
            //delete(image);
        }

        usleep(100000);
    }
    //cout << "show end------------------------------------------" << endl;
    //while(1);
}


void
MainWindow::on_start_btn()
{
    int consumerId = 1;

    consumerId = controler->addConsumer();

    controler->startConsumer(consumerId);

    std::thread playerThread(&MainWindow::showTable,this,consumerId);
    playerThread.detach();

    //controler->consumer->start();
}


void
MainWindow::on_stop_btn()
{
    controler->stopConsumer(1);
}


//void MainWindow::on_actionAdd_Stream_clicked()
//{

//}
