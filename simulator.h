#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <iostream>

#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_
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


static int durationCounter = -1;
static int jobCounter = -1;


class Simulator
{
public:
    Simulator():
        p_alpha(0.6), p_x_min(5.0), p_quantity(10000), p_min(5.0), p_max(10.0),  // pareto parameters
        z_alpha(0.6), z_quantity(100), z_min(0), z_max(100),  // zipf parameters
        counter(0)
    {
        durations = pareto(p_alpha, p_x_min, p_quantity, p_min, p_max);
        jobs = zipf(z_alpha, z_quantity );
    }

    ~Simulator();


    long getTimer()
    {
        //cout << simulatorCounter+1<<endl;
        return 1000*durations[++durationCounter];
    }


    std::string getNextURI()
    {
        stringstream ss;
        string str;
        ss<<jobs[++jobCounter];
        ss>>str;
        std::string destURI("/video");
        destURI.append("/");
        destURI.append(str);
        destURI.append(":localhost:6363");
        return destURI;
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

};

#endif //_SIMULATOR_H_
