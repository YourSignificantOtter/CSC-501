#ifndef LAB1_H_
#define LAB1_H_

#define RAND_MAX 0x7FFF

int rand(void);
//void srand(unsigned int);

double pow(double x, double y);
double log(double x);
double expdev(double lambda);

void setschedclass(int sched_class);
int getschedclass();

#endif
