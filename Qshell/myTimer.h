#ifndef MYTIMER_H
#define MYTIMER_H

#include <QObject>

class MyTimer : public QObject

{

    Q_OBJECT

public:

    friend class MainWindow;


    MyTimer( QObject  *parent = 0 );

    //virtual ~MyTimer();
    ~MyTimer();

    int startMyTimer(int interval, int jobid);


signals:
    void myTimeout( int k );


protected:
    void timerEvent( QTimerEvent *event );

    int m_nTimerId;
    int jobId;

    struct timeval t_1, t_2;
    long lt_1,lt_2;

};



#endif // MYTIMER_H
