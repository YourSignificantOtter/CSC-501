static	long	randx = 1;

srand(unsigned x)
{
	randx = x;
}

rand()
{
	randx = ((randx*1103515245 + 12345)>>16) & 077777;
	return(randx);
}
