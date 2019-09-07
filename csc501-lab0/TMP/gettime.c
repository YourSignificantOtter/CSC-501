/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>

#include "lab0.h"
#include <proc.h>

extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
SYSCALL	gettime(long *timvar)
{
    /* long	now; */

	/* FIXME -- no getutim */

	if(sysCallCounterFlag == TRUE)
	{
		sysCallCounters[currpid][GETTIME_NUM]++;
		sysCallExecTimes[currpid][GETTIME_NUM] += 0; //This process will not take 1 ms;
	}

    return OK;
}
