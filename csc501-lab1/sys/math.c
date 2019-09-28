#include "lab1.h"

double pow(double x, double y)
{
	if(y == 0)
		return 1.0;
	double toRet = x;
	int i = 0;
	for(; i < y; i++)
		toRet *= x;	
	return toRet;
}

double log(double x)
{

	double tay = -0.5;
	int i = 0;
	for(i = 1; i < 21; i++)
	{
		if(i % 2 == 0)
			tay += pow(x - (double)1.0, (double)i) / (double) i;
		else
			tay -= pow(x - (double)1.0, (double)i) / (double) i;
	}
	return tay;
}

double expdev(double lambda)
{
	double r;
	do
		r = (double) rand() / (double) RAND_MAX;
	while(r == 0.0);
	return ((double) 1.0/ (double)lambda) * log(r);
}
