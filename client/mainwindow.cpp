#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simulator.h"
#include "myTimer.h"
#include "logger.h"
#include "renderer.h"

#include <thread>
#include <math.h>
#include <time.h>

//#define WIDTH 1080
//#define HEIGHT 720
#define WIDTH 640
#define HEIGHT 480

#define HOST "10.103.246.164"
#define PORT 6363

#define _FRAME_RATE_ 30*1000    //30ms

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    controler(new Controler),
    space_(5),
    simulator(new Simulator)/*,
    timer(new QTimer(this))*/,
    mytimer_(NULL)
{
    ui->setupUi(this);

    pixmap = QPixmap(WIDTH,HEIGHT);
    bFit = true;
}

MainWindow::~MainWindow()
{
    if( ui )
        delete ui;
    if( manager_ )
        delete manager_;
}


void
MainWindow::showStream( int id )
{
#ifdef __SHOW_CONSOLE_
    cout << "[Player] Start " << id << endl;
#endif

    //controler->lock();
    Consumer *consumer = controler->getConsumer(id);

    if(consumer == NULL)
    {
        //controler->unlock();
        cout << "Consumer "<< id << " not started" << endl;
        return ;
    }
    while( consumer->getstatus() == Consumer::Status::STOPED)
        usleep(1000);
    unsigned char *tmp = consumer->player_->bmp_frameBuf_;

    //controler->unlock();
    QImage *image = new QImage(tmp,WIDTH,HEIGHT,QImage::Format_RGB888);
    map<int,QLabel*>::iterator iter = mapLabels.find(id);
    QLabel *label = NULL;
    unsigned int rate, sleepTime;

    if(iter==mapLabels.end())
    {
        return;
    }
    else
    {
        label = iter->second;
    }

    while(  1 /*++i<= 202*/ )
    {
        //time_t t1 = time(NULL);
        //struct timeval t_start;
        //gettimeofday(&t_start, NULL);

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
            rate = controler->getConsumer(id)->player_->refresh(1);
            controler->getConsumer(id)->player_->unlock();
            //return;

            int num, width, height, label_x, label_y;

            num = controler->consumerNumber_;
            //num = consumerSmltId;

            int base_ = ceil(sqrt(num));
            if (base_ == 0) break;

            width = (ui->labelWidget->width())/base_;
            height = ( ui->labelWidget->height() )/base_;

            int i = 0, labelId = 0;
            map<int,Consumer*>::iterator iter;
            for ( iter = controler->consumersMap_.begin(); iter != controler->consumersMap_.end(); iter++ )
            {
                if( iter->first == id )
                    break;
                i++;
            }
            labelId = i;
            label_x = ( (labelId) % base_ )  *width;
            label_y = ( (labelId) / base_ )  *height;

//            cout    << endl
//                    << width << "*" << height << endl
//                    << x << "*" << y << endl
//                    << ( (id%base_) -1 ) << "*" << ( (id-1) / base_ ) << endl
//                    << id << "*"  << base_
//                    << endl;

            pixmap = QPixmap::fromImage(image->scaled(width, height, Qt::KeepAspectRatio));

            label->setPixmap(pixmap);
            //label->setText(QString::number(id));
            label->resize(width,height);
            label->move(label_x,label_y);

//            cout    << endl
//                    << "Widget: " << ui->labelWidget->width() << "*"<< ui->labelWidget->height() << endl
//                    << "Label : " << label->width() << "*"<< label->height() << " " << label->x() << "*"<< label->y()
//                    << endl << endl;

            label->show();
        }
        controler->unlock();
        //time_t t2 = time(NULL);
        //struct timeval t_end1;
        //gettimeofday(&t_end1, NULL);

        if( rate < 5 )
            rate = 25  *1000;
//        else if( rate >= 5 &&rate < 10 )
//            sleepTime = 1000;
        else
            sleepTime = 0;
        //sleepTime = rate > 5 ? 10*1000 : 30*1000;
        //cout << rate << endl;
        usleep(sleepTime);
        //time_t t3 = time(NULL);
//        struct timeval t_end2;
//        gettimeofday(&t_end2, NULL);
//        long start = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000;
//        long end1 = ((long)t_end1.tv_sec)*1000+(long)t_end1.tv_usec/1000;
//        long end2 = ((long)t_end2.tv_sec)*1000+(long)t_end2.tv_usec/1000;
//        cout << "time1 " << end1-start << " time2 " << end2 - end1 << endl;
    }
    label->close();
}


bool
MainWindow::onFrameRefresh( const int id, const unsigned char *pframe,
                            const unsigned int width, const unsigned int height  )
{
    refreshLabel(id,pframe,width, height);
}

void
MainWindow::onFrameDelivered( const unsigned char *frame )
{
    //refreshLabel(id,pframe,width, height);
}

void
MainWindow::closeEvent(QCloseEvent *event)
{
    if( manager_ )
        delete manager_;
    event->accept();
}

/*
void
MainWindow::on_start_btn_clicked()
{
    //addStream("/video:10.103.240.100:6363");
    string prefix("/com/monitor/location1/stream0/video:localhost:6363");
    //addStream("/video:localhost:6363");
    addStream(prefix);
}
*/
void
MainWindow::on_start_btn_clicked()
{
    manager_ = &(MMNdnLibrary::getSharedInstance());
    std::string remoteStreamPrefix = "/com/monitor/location1/stream0/video";
    std::string threadName = "/com/monitor/location1/stream0/video";
    MediaStreamParams params;
    params.type_ = MediaStreamParams::MediaStreamTypeVideo;
    params.streamName_ = remoteStreamPrefix;

    GeneralParams generalParams;
    generalParams.prefix_ = remoteStreamPrefix;
    generalParams.host_ = "10.103.246.164";
    generalParams.portNum_ = 6363;

    GeneralConsumerParams consumerParams;
    consumerParams.interestLifetime_ = 30;
    consumerParams.bufferSlotsNum_ = 20;
    consumerParams.slotSize_ = 8800;
    consumerParams.jitterSizeMs_ = 0;

    RendererInternal *renderer = new RendererInternal(this);
    renderer_.push_back(renderer);
    refreshRenderer();

    manager_->addRemoteStream(remoteStreamPrefix,
                             threadName,
                             params,
                             generalParams,
                             consumerParams,
                             renderer);
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
MainWindow::on_simulate_waiting( int simuId )
{
    //if(mytimer_ != NULL) delete mytimer_;
//    gettimeofday(&t_1, NULL);
//    lt_1 = ((long)t_1.tv_sec)*1000+(long)t_1.tv_usec/1000;
//    cout <<endl<< "Fetching time: " << lt_1 - lt_2 << endl;


    stopStream(simuId);

    long duration = simulator->getDuration();

    clock_t start;//, finish;
    start = clock();

    //finish = start + duration;

    //controler->stopConsumer(consumerSmltId);

    struct timeval t_1;
    long lt_1;
    gettimeofday(&t_1, NULL);
    lt_1 = ((long)t_1.tv_sec)*1000+(long)t_1.tv_usec/1000;

    cout << endl
         << "***********************************************************" << endl
         << "[Simulator] " << simuId <<"-Stoped "<< controler->consumerNumber_ << "-activating" <<endl
         << "Waiting:   " << duration << "ms" << endl
         << "TimeStamp: " << lt_1 << endl
         << "***********************************************************"
         << endl << endl;

   // usleep(1000);


    //timer->disconnect();
    //cout << "disconnect fetch"<<endl;
    //connect(timer,SIGNAL(timeout()),this,SLOT(on_simulate_btn_clicked()));

    //timer->start(duration);

    mytimer_ = new MyTimer();
    mytimer_->disconnect();
    connect(mytimer_,&MyTimer::myTimeout,this,&MainWindow::on_simulate_fetching);
//    gettimeofday(&t_1, NULL);
//    lt_1 = ((long)t_1.tv_sec)*1000+(long)t_1.tv_usec/1000;
    mytimer_->startMyTimer(duration, simuId);
}

static int simucounter=0;


void
MainWindow::on_simulate_fetching( int id )
{
    //if(mytimer_ != NULL) delete mytimer_;
//    gettimeofday(&t_2, NULL);
//    lt_2 = ((long)t_2.tv_sec)*1000+(long)t_2.tv_usec/1000;
//    cout <<endl<<"Waiting time: " << lt_2-lt_1 << endl;


    std::string nextURI;
    long duration;

    duration = simulator->getDuration();
    nextURI = simulator->getNextURI();

    clock_t start, finish;
    start = clock();

    finish = start + duration;

    int simuId = addStream(nextURI);

    struct timeval t_1;
    long lt_1;
    gettimeofday(&t_1, NULL);
    lt_1 = ((long)t_1.tv_sec)*1000+(long)t_1.tv_usec/1000;

    cout << endl
         << "###########################################################"<< endl
         << ++simucounter << endl
         << "[Simulator] " << simuId << "-Started (after " << id << " stoped) " << controler->consumerNumber_ << "-activating" <<endl
         << "Fetching : " << nextURI << endl
         << "Time     : " << duration << "ms" << endl
         << "TimeStamp: " << lt_1 << endl
         << "###########################################################"
         << endl << endl;

    //timer->disconnect();
    //cout << "disconnect wait"<<endl;
    //connect(timer,SIGNAL(timeout()),this,SLOT(on_simulate_wait()));

    //mytimer = new MyTimer(this);
    mytimer_ = new MyTimer();
    mytimer_->disconnect();
    connect(mytimer_,&MyTimer::myTimeout,this,&MainWindow::on_simulate_waiting);
//    gettimeofday(&t_2, NULL);
//    lt_2 = ((long)t_2.tv_sec)*1000+(long)t_2.tv_usec/1000;
    mytimer_->startMyTimer(duration, simuId);

    //timer->start(duration);
}


void
MainWindow::on_simulate_btn_clicked()
{
    on_simulate_fetching(0);
}


int
MainWindow::addStream( std::string stream_/*QString stream*/ )
{
    LOG(INFO)<<"Adding stream: \"" << stream_ << "\"";

    QString stream = QString::fromStdString(stream_);
    QStringList list = stream.split(":");

    if (list.size() < 2)
    {
        return -1;
    }
    QString prefix = list[0].simplified();
    //QString host = list[1].simplified();
    //int port = (list.size() > 2) ? list[2].toInt() : 6363;

//    std::cout << "Prefix: " << prefix.toStdString() << std::endl
//              << "Face  : " << host.toStdString()
//              << ":" << port << std::endl;

    int consumerId = 1;

    //controler->createFace(host,port);
//    consumerId = controler->addConsumer( prefix.toStdString() );
//    controler->startConsumer(consumerId);

    consumerId = controler->addStream(prefix.toStdString());

    //int num = controler->consumerNumber_;
    QLabel *label = new QLabel(this->ui->labelWidget);
    mapLabels.insert(pair<int,QLabel*>(consumerId,label));//.push_back(label);

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
    {
        LOG(ERROR) << "Stop Consumer failed! consumerId=" << id << endl;
        return -1;
    }
    return 1;
}


std::shared_ptr<Consumer>
MainWindow::getConsumer( int id )
{
    std::shared_ptr<Consumer> consumer;
    map<int, std::shared_ptr<Consumer> >::iterator it;
    it = mapConsumers.find(id);
    if( it != mapConsumers.end() )
        consumer = it->second;
    return consumer;
}

QLabel*
MainWindow::getLabel( int id )
{
    QLabel *label;
    std::map<int,QLabel*>::iterator it;
    it = mapLabels.find(id);
    if( it != mapLabels.end() )
        label = it->second;
    return label;
}

void
MainWindow::refreshLabel(const int id, const unsigned char *pframe,
                  const unsigned int width, const unsigned int height)
{
    QLabel *label;
    label = getLabel(id);

    int num, label_x, label_y;

    num = activedConsumerNum_;
    //num = consumerSmltId;

    int base_ = ceil(sqrt(num));
    if (base_ == 0) return;

//    width = (ui->labelWidget->width())/base_;
//    height = ( ui->labelWidget->height() )/base_;

    int i = 0, labelId = 0;
    map<int,Consumer*>::iterator iter;
    for ( iter = controler->consumersMap_.begin(); iter != controler->consumersMap_.end(); iter++ )
    {
        if( iter->first == id )
            break;
        i++;
    }
    labelId = i;
    label_x = ( (labelId) % base_ )  *width;
    label_y = ( (labelId) / base_ )  *height;

    QImage *image = new QImage(pframe,width,height,QImage::Format_RGB888);
    pixmap = QPixmap::fromImage(image->scaled(label_width, label_height, Qt::KeepAspectRatio));

    label->setPixmap(pixmap);
    //label->setText(QString::number(id));
    label->resize(label_width,label_height);
    label->move(label_x,label_y);

//            cout    << endl
//                    << "Widget: " << ui->labelWidget->width() << "*"<< ui->labelWidget->height() << endl
//                    << "Label : " << label->width() << "*"<< label->height() << " " << label->x() << "*"<< label->y()
//                    << endl << endl;

    label->show();
}

int
MainWindow::refreshRenderer()
{
    //int rNo = renderer_.size();
    vector<RendererInternal*>::iterator it;
    RendererInternal* render;
    for( it = renderer_.begin(); it != renderer_.end(); ++it )
    {
        render = *it;
        QPoint point(0,0);
        int width = 640;//this->ui->labelWidget->width();
        int height = 480;//this->ui->labelWidget->height();
        render->refresh(this, point, width, height );
    }
}
