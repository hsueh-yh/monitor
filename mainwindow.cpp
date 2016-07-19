#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simulator.h"

#include <thread>
#include <math.h>
#include <time.h>

//#define WIDTH 1080
//#define HEIGHT 720
#define WIDTH 640
#define HEIGHT 480

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    controler(new Controler),
    space_(5),
    simulator(new Simulator),
    timer(new QTimer(this)),
    consumerSmltId(0)
{
    ui->setupUi(this);

    pixmap = QPixmap(WIDTH,HEIGHT);
    bFit = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void
MainWindow::showStream( int id )
{
#ifdef __SHOW_CONSOLE_
    cout << "[Player] Start " << id << endl;
#endif

    controler->lock();
    Consumer *consumer = controler->getConsumer(id);

    if(consumer == NULL)
    {
        controler->unlock();
        cout << "Consumer "<< id << " not started" << endl;
        return ;
    }
    while( consumer->getstatus() == Consumer::Status::STOPED)
        usleep(1000);
    unsigned char *tmp = consumer->player_->bmp_frameBuf_;

    controler->unlock();
    QImage *image = new QImage(tmp,WIDTH,HEIGHT,QImage::Format_RGB888);
    map<int,QLabel*>::iterator iter = labelMap.find(id);
    QLabel *label = NULL;

    if(iter==labelMap.end())
    {
        return;
    }
    else
    {
        label = iter->second;
    }

    while(  1 /*++i<= 202*/ )
    {
        controler->lock();
        if( NULL == controler->getConsumer(id))
        {
            controler->unlock();
            break;
        }
        if( consumer->getstatus() != Consumer::Status::STARTED)
        {
            controler->unlock();
            break;
        }
        if( tmp == NULL || label == NULL )
        {
            controler->unlock();
            label->setText("Waiting ...");
        }
        else
        {
            if( NULL == controler->getConsumer(id)) break;
            if( consumer->getstatus() != Consumer::Status::STARTED) break;
            controler->getConsumer(id)->player_->lock();
            controler->getConsumer(id)->player_->refresh();
            controler->getConsumer(id)->player_->unlock();
            //return;

            int num, width, height, x, y;

            num = controler->consumerNumber_;
            //num = consumerSmltId;

            int base_ = ceil(sqrt(num));
            if (base_ == 0) break;

            width = (ui->labelWidget->width())/base_;
            height = ( ui->labelWidget->height() )/base_;

            x = ( (id-1) % base_ ) * width;
            y = ( (id-1) / base_ ) * height;

//            cout    << width << "*" << height << endl
//                    << x << "*" << y << endl
//                    << ( (id%base_) -1 ) << "*" << ( (id-1) / base_ ) << endl
//                    << id << "*"  << base_;

            pixmap = QPixmap::fromImage(image->scaled(width, height, Qt::KeepAspectRatio));

            label->setPixmap(pixmap);
            label->resize(width,height);
            label->move(x,y);

//            cout    << endl
//                    << "Widget: " << ui->labelWidget->width() << "*"<< ui->labelWidget->height() << endl
//                    << "Label : " << label->width() << "*"<< label->height() << " " << label->x() << "*"<< label->y()
//                    << endl << endl;

            label->show();
        }
        controler->unlock();
        usleep(40 * 1000);
    }
    label->close();
}


void
MainWindow::on_start_btn_clicked()
{
    //addStream("/video:10.103.240.100:6363");
    addStream("/video:localhost:6363");
}


void
MainWindow::on_stop_btn_clicked()
{
    controler->stopConsumer(controler->consumerNumber_);
}


void
MainWindow::on_add_btn_clicked()
{
    addStreamDialog = new AddStreamDialog(this);
    connect(addStreamDialog, &AddStreamDialog::addedStream, this, &MainWindow::addStream);

    addStreamDialog->show();
}


void
MainWindow::on_simulate_wait()
{
    stopStream(consumerSmltId);

    long duration = simulator->getTimer();

    clock_t start, finish;
    start = clock();

    finish = start + duration;

    //controler->stopConsumer(consumerSmltId);

    cout << endl
         << "[Simulator] Waiting "
         << duration << "ms"
         << endl;

   // usleep(1000);


    timer->disconnect();
    //cout << "disconnect fetch"<<endl;
    connect(timer,SIGNAL(timeout()),this,SLOT(on_simulate_btn_clicked()));

    timer->start(duration);
}


void
MainWindow::on_simulate_btn_clicked()
{
    std::string nextURI;
    long duration;

    nextURI = simulator->getNextURI();
    duration = simulator->getTimer();

    clock_t start, finish;
    start = clock();

    finish = start + duration;

    cout << endl
         << "[Simulator] Fetching " << nextURI<< " "
         << duration << "ms"
         << endl;
    consumerSmltId = addStream(nextURI);

    timer->disconnect();
    //cout << "disconnect wait"<<endl;
    connect(timer,SIGNAL(timeout()),this,SLOT(on_simulate_wait()));

    timer->start(duration);
}


int
MainWindow::addStream( std::string stream_/*QString stream*/ )
{
    //cout << "Added Stream"<<endl;
    QString stream = QString::fromStdString(stream_);
    QStringList list = stream.split(":");

    if (list.size() < 2)
    {
        return -1;
    }
    QString prefix = list[0].simplified();
    QString host = list[1].simplified();
    int port = (list.size() > 2) ? list[2].toInt() : 6363;

//    std::cout << "Prefix: " << prefix.toStdString() << std::endl
//              << "Face  : " << host.toStdString()
//              << ":" << port << std::endl;

    int consumerId = 1;

    //controler->createFace(host,port);

//    consumerId = controler->addConsumer( prefix.toStdString() );

//    controler->startConsumer(consumerId);

    consumerId = controler->addStream(prefix.toStdString());

    int num = controler->consumerNumber_;
    QLabel *label = new QLabel(this->ui->labelWidget);
    labelMap.insert(pair<int,QLabel*>(consumerId,label));//.push_back(label);

    std::thread playerThread(&MainWindow::showStream,this,consumerId);

    playerThread.detach();

    controler->playerId_.insert(pair<int,pthread_t>(consumerId,playerThread.native_handle()));
    //controler->consumer->start();
    return consumerId;
}


int
MainWindow::stopStream(int id)
{
//    map<int,pthread_t>::iterator iter;

//    iter = controler->playerId_.find(id);

//    if( iter != controler->playerId_.end() )
//    {
//        pthread_kill(iter->second,SIGTERM);
//        cout<<"Kill player" << endl;
//        controler->playerId_.erase(iter);
//    }

    if ( -1 == controler->stopConsumer(id))
        cout << "Stop Consumer failed! " << endl;
}
