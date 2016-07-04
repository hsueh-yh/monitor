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

    //connect(ui->start_btn,SIGNAL(clicked()),this,SLOT(on_start_btn_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
/*
void MainWindow::paintEvent(QPaintEvent *event)
{
    QPixmap fitPixmap = pixmap.scaled(ui->txlabel->width(),ui->txlabel->height(), Qt::KeepAspectRatio);
    ui->txlabel->setPixmap(fitPixmap);
    //cout << ui->txlabel->width()<< " " << ui->txlabel->height() << " "<< fitPixmap.width() << " " << fitPixmap.height()<< endl;
    ui->txlabel->show();


//        QPainter painter(this);

//        if(bFit)
//        {
//                QPixmap fitPixmap = pixmap.scaled(640,480QMainWindow::width(),QMainWindow::height(), Qt::KeepAspectRatio);
//                cout << fitPixmap.width() << " " << fitPixmap.height()<< "****************************************************************" << endl;
//                painter.drawPixmap(0, 0, fitPixmap);
//        }
//        else
//                painter.drawPixmap(0, 0, pixmap);

}

void MainWindow::setPixmap(const uchar* buf)
{
        pixmap.loadFromData(buf, 640*480*3/2+54, "bmp", NULL);
        update();
}
*/

void MainWindow::on_start_btn_clicked()
{
//    int id = 0;
    controler->addConsumer();
//    QPainter *painter = new QPainter();
//    painters.push_back(painter);
    //painters = new QPainter(620,480);

    cout << "MainWindow: " << (int)getpid() << "-"
         << std::this_thread::get_id() << " ";

    //controler->player_->init();
    cout << "player init done." << endl;

    controler->startConsumer(1);

    controler->player_->init(controler->consumer->frameBuffer_);

    std::thread playerThread([&]
        {
           int i =  0;

           cout << "player thread start." << endl;

           while( ++i  /*<= 202*/ ){

            unsigned char *tmp = controler->player_->bmp_frameBuf_;
            image = new QImage(tmp,640,480,QImage::Format_RGB888);
            if( tmp == NULL )
                ui->txlabel->setText("helloworld");
            else
            {
                //setPixmap(tmp);
                //image = new QImage(tmp,640,480,QImage::Format_RGB888);
                controler->player_->getPixmap();
                pixmap = QPixmap::fromImage(*image);
                ui->txlabel->setPixmap(pixmap);
                ui->txlabel->show();
                //delete(image);
            }

//            if(tmp == NULL)
//                ui->txlabel->setText("helloworld");
//            else
//                ui->txlabel->setPixmap(pixmap);

//                ui->txlabel->show();

            //QPixmap *pixmap = controler->consumer->player_->getPixmap();
            //ainters->drawPixmap(0, 0,640,480, *pixmap);
//                vector<QPainter*>::iterator iter;
//                iter = controler->consumers.at(id);
//                if(iter != controler->consumers.end())
//                {
//                    iter->
//                }

            usleep(100000);
           }
           //cout << "show end------------------------------------------" << endl;
           //while(1);
        });
        playerThread.detach();

        //controler->consumer->start();
}


void MainWindow::on_stop_btn_clicked()
{

}


void MainWindow::on_actionAdd_Stream_clicked()
{

}
