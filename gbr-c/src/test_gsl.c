#include <stdio.h>
#include <gsl-2.5/statistics/gsl_statistics.h>

int main(){

	double data[] = {1, 2, 3, 4, 5};

	double mean = 0;
	mean = gsl_stats_mean(data, 1 ,5);

	printf(mean);

	return 0;
}

