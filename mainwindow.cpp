#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simulator.h"

#include <thread>
#include <math.h>

//#define WIDTH 1080
//#define HEIGHT 720
#define WIDTH 640
#define HEIGHT 480

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    controler(new Controler),
    space_(5)
{
    ui->setupUi(this);
   // ui->txlabel->resize(WIDTH,HEIGHT);
    //layout = new QVBoxLayout( this );
    //ui->centralWidget->setLayout(layout);

    pixmap = QPixmap(WIDTH,HEIGHT);
    bFit = true;

    //ui->txlabel->resize(WIDTH,HEIGHT);

//    connect(ui->start_btn,SIGNAL(clicked()),this,SLOT(on_start_btn()));
//    connect(ui->stop_btn,SIGNAL(clicked()),this,SLOT(on_stop_btn()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void
MainWindow::showStream( int id )
{
    cout << "Start Player " << id << endl;

    Consumer *consumer = controler->getConsumer(id);

    if(consumer == NULL)
    {
        cout << "Consumer "<< id << " not started" << endl;
        return ;
    }

    unsigned char *tmp = consumer->player_->bmp_frameBuf_;

    QImage *image = new QImage(tmp,WIDTH,HEIGHT,QImage::Format_RGB888);
    map<int,QLabel*>::iterator iter = labelMap.find(id);
    QLabel *label = NULL;
    if(iter==labelMap.end())
    {
//        return;
    }
    else
    {
        label = iter->second;
    }

    //layout->addWidget(label);
    int i = 0;
    //label->move((id-1)*width,(id-1)*height);
    //label->move(0,0);
    while( ++i  /*<= 202*/ )
    {
        //unsigned char *tmp = controler->getConsumer(id)->player_->bmp_frameBuf_;
        //image = new QImage(tmp,WIDTH,HEIGHT,QImage::Format_RGB888);
        if( tmp == NULL || label == NULL )
        {
            label->setText("Waiting ...");
        }
        else
        {
            //setPixmap(tmp);
            //image = new QImage(tmp,WIDTH,HEIGHT,QImage::Format_RGB888);
            controler->getConsumer(id)->player_->refresh();
            //return;

            int num, width, height, x, y;

            num = controler->consumerNumber_;
            int base_ = ceil(sqrt(num));

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
            //std::cout << width << "*" << height << std::endl;
            //delete(image);
        }

        usleep(40 * 1000);
    }
    //cout << "show end------------------------------------------" << endl;
    //while(1);
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
    controler->stopConsumer(1);
}


void
MainWindow::on_add_btn_clicked()
{
    addStreamDialog = new AddStreamDialog(this);
    connect(addStreamDialog, &AddStreamDialog::addedStream, this, &MainWindow::addStream);

    addStreamDialog->show();
}


void
MainWindow::simulatorWork(int index, double duration/*, const boost::system::error_code& e*/ )
{
    int idx = index;
    if(counter >= p_quantity)
    {
        cout << "ended" << endl;
        return;
    }
    if(counter%2 != 0)
        idx = -1;
    std::string jobstr("/video");
    int consumerId;

    std::cout << "Do job: " << counter + 1 << " Dest:"<< idx << " time(s):" << (int)(duration) << std::endl;
    if( idx >= 0 )
    {
        //jobstr.append(std::to_string(idx));
        jobstr.append(":10.103.240.100:6363");
        cout << jobstr << endl;
        //consumerId = controler->addStream(jobstr);
        addStream(jobstr);
    }
    else
    {
        cout << "stop" << endl << endl;
        //controler->stopConsumer(consumerId);
        //mainwindow_->sto(jobstr);
    }
    timer->expires_from_now(std::chrono::microseconds((int)(duration*1000*1000)));
    timer->async_wait(bind(&MainWindow::simulatorWork, this, jobs[counter], durations[counter]));
    ++counter;
}


void
MainWindow::on_simulate_btn_clicked()
{/*
    int count, min, max;
    double *random_pareto = pareto((double)1/3, 5.0, count, min, max);
    double *random_zipf = zipf(0.6, 100);

    for ( int i = 0; i < count; i++ )
    */


    //std::thread simulatorThread(&Simulator::start,simulator);
    std::thread simulatorThread([&]
    {
        boost::asio::io_service io;
        timer= new boost::asio::steady_timer(io);
        durations = pareto(p_alpha, p_x_min, p_quantity, p_min, p_max);
        jobs = zipf(z_alpha, z_quantity );

        simulatorWork(jobs[counter], durations[counter]);

//        boost::asio::io_service io;
//        Simulator *simulator = new Simulator(this,io);
//        simulator->start();
//        io.run();
//        cout << "Simulator ended" << endl;
    });

    simulatorThread.detach();
}


//void
//MainWindow::startStream(string stream)
//{
//    addStream(stream);
//}

void
MainWindow::addStream( std::string stream_/*QString stream*/ )
{
    cout << "Added Stream"<<endl;
    QString stream = QString::fromStdString(stream_);
    QStringList list = stream.split(":");

    if (list.size() < 2)
    {
        return ;
    }
    QString prefix = list[0].simplified();
    QString host = list[1].simplified();
    int port = (list.size() > 2) ? list[2].toInt() : 6363;

    std::cout << "Prefix: " << prefix.toStdString() << std::endl
              << "Face  : " << host.toStdString()
              << ":" << port << std::endl;

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

    //controler->consumer->start();
}
