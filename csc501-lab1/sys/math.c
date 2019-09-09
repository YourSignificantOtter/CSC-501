//rand is in lib/libxc/rand.c
#include "stdlib.h"
double pow(double x, double y)
{
	if(y == 0)
		return 1;
	double toRet = x;
	int i = 0;
	for(; i < y; i++)
		toRet *= x;	
	return toRet;
}

double log(double x)
{
	double tay = x;
	double i = 2.0;
	for(; i < 20.0; i += 1.0)
	{
		if((int)i % 2 == 0) //Even
			tay -= pow(x, i)/i;
		else //Odd
			tay += pow(x, i)/i;
	}
	return tay;
}

double expdev(double lambda)
{
	double dummy;
	do
		dummy = (double) rand() / RAND_MAX;
	while(dummy == 0.0);
	return -log(dummy) / lambda;
}
