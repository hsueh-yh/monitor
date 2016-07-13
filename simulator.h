#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


#define _SIMULATOR_ACCURACY_ 1000000

/*
 *	Zipf distribution function:	F(x) = 1 - ( X_m / x )^a
 *	@alpha: 
 *	@x_min :	
 */
double *pareto( double alpha, double x_min, int n, double min, double max )
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
	printf("\n\n");
	
	for ( int i = 0; i < n; i++ )
	{
		x = (int)rand()%_SIMULATOR_ACCURACY_;
		x /= _SIMULATOR_ACCURACY_;
		//the inverse function of Pareto distribution function
		double k = x_min * (double)pow( (1-x), (-1.0/alpha) );
		//printf("%lf ", k);

		// if this random number is out of range
		if ( k >= min && k <= max )
		{
			result[i] = k;
			//printf("%d ", (int)floor(result[i]) );
		}
		else
		{
			i--;
			/*
			double floor_k = floor(k);
			double r = (int)floor_k % size;
			r += (k-floor_k);
			result[i] = min + r;
			//printf("%d ", (int)floor(result[i]) );
			*/
		}
	}

	return result;
}


/*
 *	Zipf distribution law:	P(x) = C / (x^a);
 *	
 */
double *zipf( double alpha, int n )
{
	double c, x;

	for ( int i = 1; i <= n; i++ )
	{
		c += ( 1/pow(i,alpha));
	}
	c = 1/c;

	double *result = (double*) malloc ( sizeof(double) * n );

	for ( int i = 0; i < n; i++ )
	{
		x = (int)rand()%_SIMULATOR_ACCURACY_;
		x /= _SIMULATOR_ACCURACY_;

		result[i] = pow ( ( c / x ), ( 1.0 / alpha ) );
		//printf("%lf ", result[i]);
	}

	return result;
}