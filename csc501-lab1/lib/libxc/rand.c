#define RAND_MAX	2147483646
static	long	randx = 1;

srand(unsigned x)
{
	randx = x;
}

rand()
{
	randx = ((randx*1103515245 + 12345)>>16) & 077777;
	randx > RAND_MAX ? randx = RAND_MAX : randx;
	return(randx);
}
