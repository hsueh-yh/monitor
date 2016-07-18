#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include "controler.h"
#include "mainwindow.h"


#define _SIMULATOR_ACCURACY_ 1000000

/*
 *	Zipf distribution function:	F(x) = 1 - ( X_m / x )^a
 *	@alpha:
 *	@x_min :
 */
static double *pareto( double alpha, double x_min, int n, double min, double max )
{
    double x;
    double *result;
    int size = max - min;

    result = (double*) malloc ( sizeof(double) * n );

    srand((unsigned)time(NULL));
    for ( int i = 0; i < n; i++ )
    {
        result[i] = 0.0;
    }

    for ( int i = 0; i < n; i++ )
    {
        x = (double)(rand()%_SIMULATOR_ACCURACY_) /_SIMULATOR_ACCURACY_;

        //the inverse function of Pareto distribution function
        double k = x_min * (double)pow( (1-x), (-1.0/alpha) );

        // if this random number is out of range
        if ( k >= min && k <= max )
        {
            result[i] = k;
        }
        else
        {
            i--;
        }
    }

    return result;
}


/*
 *	Zipf distribution law:	P(x) = C / (x^a);
 *
 */
static int *zipf( double alpha, int n )
{
    double c, x;

    for ( int i = 1; i <= n; i++ )
    {
        c += ( 1/pow(i,alpha));
    }
    c = 1/c;

    //printf("\nC = %lf n = %d\n",c, n);

    int *result = (int*) malloc ( sizeof(int) * n );

    double sum = 0.0;

    srand((unsigned)time(NULL));
    for ( int i = 0; i < n; i++ )
    {
        x = (double)(rand()%_SIMULATOR_ACCURACY_) /_SIMULATOR_ACCURACY_;

        int j = 0;
        sum = 0.0;
        while ( ++j <= n )
        {
            sum += c / pow( j, alpha );
            if ( x <= sum )
                break;
        }

        if ( j > n )
            i--;
        else
            result[i] = j;
    }

    return result;
}


static int simulatorCounter = 0;


class Simulator
{
public:
    Simulator(MainWindow* mainwindow, boost::asio::io_service& io):
        mainwindow_(mainwindow),
        controler(new Controler),
        p_alpha(0.6), p_x_min(5.0), p_quantity(10), p_min(5.0), p_max(10.0),  // pareto parameters
        z_alpha(0.6), z_quantity(100), z_min(0), z_max(100),  // zipf parameters
        counter(0),
        timer(io)
    {
        durations = pareto(p_alpha, p_x_min, p_quantity, p_min, p_max);
        jobs = zipf(z_alpha, z_quantity );
    }

    ~Simulator();


//    long getTimer()
//    {
//        return ;
//    }


    std::string getNextURI()
    {
        std::string destURI("/video");
        destURI.append("localhost:6363");
        return destURI;
    }

    void start()
    {
        durations = pareto(p_alpha, p_x_min, p_quantity, p_min, p_max);
        jobs = zipf(z_alpha, z_quantity );

        work(jobs[counter], durations[counter]);
    }

    void work(int index, double duration/*, const boost::system::error_code& e*/ )
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
            mainwindow_->addStream(jobstr);
        }
        else
        {
            cout << "stop" << endl << endl;
            //controler->stopConsumer(consumerId);
            //mainwindow_->sto(jobstr);
        }
        timer.expires_from_now(std::chrono::microseconds((int)(duration*1000*1000)));
        timer.async_wait(bind(&Simulator::work, this, jobs[counter], durations[counter]));
        ++counter;
    }

private:
    double *durations;
    int *jobs;
    int counter;

    // pareto parameters
    double  p_alpha,
            p_x_min,
            p_min,
            p_max;
    int     p_quantity;

    // zipf parameters
    double  z_alpha,
            z_min,
            z_max;
    int     z_quantity;

    boost::asio::steady_timer timer;

    Controler *controler;
    MainWindow* mainwindow_;

};
