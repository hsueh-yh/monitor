#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simulator.h"
#include "myTimer.h"
#include "glogger.h"
#include "renderer.h"

#include <thread>
#include <math.h>
#include <time.h>

//#define WIDTH 1080
//#define HEIGHT 720
#define WIDTH 640
#define HEIGHT 480

//#define HOST "localhost"
#define HOST "10.103.244.163"
#define PORT 6363

#define _FRAME_RATE_ 30*1000    //30ms

static std::string name[4];
static int nameidx = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    space_(5),
    simulator(new Simulator)/*,
    timer(new QTimer(this))*/,
    mytimer_(NULL)
{
    ui->setupUi(this);
    name[0] = "/com/monitor/location1/stream0/video";
    name[1] = "/com/monitor/location1/stream1/video";
    name[2] = "/com/monitor/location1/stream2/video";
    name[3] = "/com/monitor/location1/stream3/video";
}

MainWindow::~MainWindow()
{
    if( ui )
        delete ui;
    if( manager_ )
        delete manager_;
}

void
MainWindow::closeEvent(QCloseEvent *event)
{
    if( manager_ )
        delete manager_;
    event->accept();
}


void
MainWindow::on_start_btn_clicked()
{
    manager_ = &(MtNdnLibrary::getSharedInstance());
    std::string remoteStreamPrefix = name[nameidx];//"/com/monitor/location1/stream0/video";
    std::string threadName = name[nameidx];//"/com/monitor/location1/stream0/video";
    MediaStreamParams params;
    params.type_ = MediaStreamParams::MediaStreamTypeVideo;
    params.streamName_ = remoteStreamPrefix;

    GeneralParams generalParams;
    generalParams.loggingLevel_ = ndnlog::NdnLoggerDetailLevelAll;
    generalParams.logPath_ = "";
    generalParams.prefix_ = remoteStreamPrefix;
    generalParams.host_ = HOST;
    generalParams.portNum_ = PORT;

    GeneralConsumerParams consumerParams;
    consumerParams.interestLifetime_ = 30;
    consumerParams.bufferSlotsNum_ = 20;
    consumerParams.slotSize_ = 8800;
    consumerParams.jitterSizeMs_ = 0;

    RendererInternal *renderer = new RendererInternal(this);
    map_str_renderer_[remoteStreamPrefix] = renderer;
    refreshRenderer();

    manager_->addRemoteStream(remoteStreamPrefix,
                             threadName,
                             params,
                             generalParams,
                             consumerParams,
                             renderer);
    ++nameidx;
}

void
MainWindow::on_stop_btn_clicked()
{
    mapRenderer::iterator it = map_str_renderer_.begin();
    while( it != map_str_renderer_.end() )
    {
        manager_->removeRemoteStream(it->first);
    }
    map_str_renderer_.clear();
}

void
MainWindow::on_add_btn_clicked()
{
    addStreamDialog = new AddStreamDialog(this);
    //connect(addStreamDialog, &AddStreamDialog::addedStream, this, &MainWindow::addStream);

    addStreamDialog->show();
}

void
MainWindow::on_simulate_waiting( int simuId )
{
    /*
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
    */
}

static int simucounter=0;

void
MainWindow::on_simulate_fetching( int id )
{
    /*
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
    */
}

void
MainWindow::on_simulate_btn_clicked()
{
    on_simulate_fetching(0);
}

int
MainWindow::refreshRenderer()
{
    //int rNo = renderer_.size();
    mapRenderer::iterator iter = map_str_renderer_.begin();
    RendererInternal *renderer;
    int idx = 0, x = 0, y = 0, width = 0, height = 0;
    for( ; iter != map_str_renderer_.end(); ++iter, ++idx )
    {

        if( calLabelParam( idx, x, y, width, height ) )
        {
            renderer = iter->second;
            QPoint point(x,y);
            //int width = 640;//this->ui->labelWidget->width();
            //int height = 480;//this->ui->labelWidget->height();
            renderer->refresh(this, point, width, height );
        }
        else
            return -1;
    }
}

bool
MainWindow::calLabelParam( int idx, int &x, int &y, int &width, int &height )
{
    int totalNum = 0;

    totalNum = map_str_renderer_.size();
    //num = consumerSmltId;

    int base_ = ceil(sqrt(totalNum));
    if (base_ == 0)
        return false;

    width = ( ui->labelWidget->width() )/base_;
    height = ( ui->labelWidget->height() )/base_;

    x = ( (idx) % base_ )  *width;
    y = ( (idx) / base_ )  *height;

//            cout    << endl
//                    << width << "*" << height << endl
//                    << x << "*" << y << endl
//                    << ( (id%base_) -1 ) << "*" << ( (id-1) / base_ ) << endl
//                    << id << "*"  << base_
//                    << endl;

    return true;
}
