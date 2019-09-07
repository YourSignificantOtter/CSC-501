/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include "lab0.h"

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */

int fakeProcess()
{
	while(1)
		;
	return 1;
}

prch(c)
char c;
{
	int i;
	sleep(5);
}

int main()
{

	long test = zfunction(0xaabbccdd);

	printsegaddress();
	
	printtos();

	int pids[5] = {0};
	resume(pids[0] = create(fakeProcess, 500, 20, "fake", 0, 0));
	resume(pids[1] = create(fakeProcess, 500, 20, "fake", 0, 0));
	resume(pids[2] = create(fakeProcess, 500, 20, "fake", 0, 0));
	resume(pids[3] = create(fakeProcess, 500, 20, "fake", 0, 0));
	resume(pids[4] = create(fakeProcess, 500, 20, "fake", 0, 0));

	printprocstks(3);

	syscallsummary_start();
	
	int pidX = 0;
	resume(pidX = create(prch, 2000, 20, "proc X", 1, 'A'));
	sleep(10);
	syscallsummary_stop();
	
	printsyscallsummary();

	return 0;
}
