#include "lab1.h"

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

	double tay = 0.0;
	double sign;
	double i;
	for(i = 1.0; i < 20.0; i += 1.0)
	{
		sign = ((int)i % 2) == 0 ? -1.0 : 1.0;
		tay += sign * (pow(x - 1.083, i) / i);
	}
	return -1 * tay;
}
/* Wrong I think?
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
*/

double expdev(double lambda)
{
	double r;
	do
		r = (double) rand() / RAND_MAX;
	while(r == 0.0);
	return (-1/lambda) * log(r);
}