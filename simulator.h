#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include "common.h"

#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_
#define _SIMULATOR_ACCURACY_ 1000000

/*
 *	Zipf distribution function:	F(x) = 1 - ( X_m / x )^a
 *	@alpha:
 *	@x_min :
 */
static double *pareto( double alpha, int n, double min, double max )
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
        double k = min * (double)pow( (1-x), (-1.0/alpha) );

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


static int durationCounter = -1;
static int jobCounter = -1;


class Simulator
{
public:
    Simulator():
        p_alpha(0.6), p_min(5.0), p_max(100.0), p_quantity(100),  // pareto parameters
        z_alpha(1.0), z_quantity(100),  // zipf parameters
        counter(0)
    {
//        durations = pareto(p_alpha, p_x_min, p_quantity, p_min, p_max);
//        jobs = zipf(z_alpha, z_quantity );
    }


    ~Simulator();


    long getTimer()
    {
        //cout << simulatorCounter+1<<endl;

        if(durationCounter >= p_quantity || durationCounter < 0 )
        {
            if(durationCounter >= p_quantity)
                delete[] durations;
            durations = pareto(p_alpha, p_quantity, p_min, p_max);
            durationCounter = 0;
#ifdef __SHOW_CONSOLE_
            viewPareto();
#endif
        }

        return 1000*durations[durationCounter++];
        //return 1000 * 5;
    }


    std::string getNextURI()
    {
        if ( jobCounter >= z_quantity || jobCounter < 0 )
        {
            if (jobCounter >= z_quantity )
                delete[] jobs;
            jobs = zipf(z_alpha, z_quantity );
            jobCounter = 0;
#ifdef __SHOW_CONSOLE_
            viewZipf();
#endif
        }

        stringstream ss;
        string str;
        //ss<<jobs[jobCounter >= z_quantity ? 0 : ++jobCounter];
        ss<<jobs[jobCounter++];
        ss>>str;
        std::string destURI("/video");
        destURI.append("/video");
        destURI.append(str);
        destURI.append(":localhost:6363");
        return destURI;
    }


    void viewPareto()
    {
        int c1, c2, c3;
        int base = (int)floor(p_max/3);
        c1 = c2 = c3 = 0;

        int i;

        cout << endl << "[Pareto]: "
             << "[alpha=" << p_alpha
             << " length=" << p_quantity
             << " min=" << p_min
             << " max=" << p_max << "]"
             << endl;
        cout << "[";
        for ( i = 0; i < p_quantity-1; i++ )
        {
            cout << durations[i] << ", ";
            (durations[i] >=0 && durations[i] <= base) ? c1++ : 0;
            (durations[i] >base && durations[i] <= base*2) ? c2++ : 0;
            (durations[i] >base*2 && durations[i] <= p_max) ? c3++ : 0;
        }
        cout << durations[i] << "]";
        cout << endl << "Count:" << endl
             << (int)p_min <<"  ~ " << base << ": " << c1 << endl
             << base << " ~ " << base*2 << ": " << c2 << endl
             << base*2 << " ~ " << p_max << ": " << c3 << endl
             << endl;
    }


    void viewZipf()
    {
        int c1,c2,c3;
        c1 = c2 = c3 = 0;

        cout << endl << "[Zipf]: "
             << "[alpha=" << z_alpha
             << " length=" << z_quantity << "]"
             <<endl;
        int i;

        cout << "[";
        for ( i = 0; i < z_quantity-1; i++ )
        {
            int k = jobs[i];
            cout << jobs[i] << " ";
            (k == 1) ? c1++ : 0;
            (k == 2) ? c2++ : 0;
            (k == 3) ? c3++ : 0;
        }
        cout << jobs[i] << "]";
        cout << endl << "Count:" << endl
             << "x = 1: " << c1 << endl
             << "x = 2: " << c2 << endl
             << "x = 3: " << c3 << endl
             << endl;
    }


private:

    double *durations;
    int *jobs;
    int counter;

    // pareto parameters
    double  p_alpha,
            p_min,
            p_max;
    int     p_quantity;

    // zipf parameters
    double  z_alpha;
    int     z_quantity;

};

#endif //_SIMULATOR_H_
