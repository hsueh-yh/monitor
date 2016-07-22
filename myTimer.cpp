
#include "myTimer.h"
#include <iostream>
#include <sys/time.h>
////////////////////////////////////////////////////
/// \brief QNewObject::QNewObject
/// \param parent
////////////////////////////////////////////////////

MyTimer::MyTimer(QObject *parent)
    //:MyTimer( parent )
{
    //m_nTimerId = startTimer(1000);
}

MyTimer::~MyTimer()
{
    if ( m_nTimerId != 0 )
        killTimer(m_nTimerId);
    gettimeofday(&t_2, NULL);
    lt_2 = ((long)t_2.tv_sec)*1000+(long)t_2.tv_usec/1000;
//    std::cout << "Timer " << m_nTimerId << " Killed"
//              << " by " << lt_2 - lt_1 << "ms"
//              << std::endl << std::endl;
}

int MyTimer::startMyTimer(int interval, int jobid )
{
    jobId = jobid;

    m_nTimerId = startTimer(interval);

    gettimeofday(&t_1, NULL);
    lt_1 = ((long)t_1.tv_sec)*1000+(long)t_1.tv_usec/1000;
    //cout <<endl<<"Waiting time: " << lt_2-lt_1 << endl;

    //std::cout << "Timer " << m_nTimerId << " started" << std::endl;
}

void MyTimer::timerEvent( QTimerEvent *event )
{
    //std::cout << "timeEvent " << jobId<<std::endl;
    emit(myTimeout(jobId));
    this->disconnect();
    delete this;
    //qDebug( "timer event, id %d",event->timerId() );
}

