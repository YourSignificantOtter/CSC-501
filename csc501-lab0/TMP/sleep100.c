/* sleep100.c - sleep100 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>

#include "lab0.h"

/*------------------------------------------------------------------------
 * sleep100  --  delay the caller for a time specified in 1/100 of seconds
 *------------------------------------------------------------------------
 */
SYSCALL sleep100(int n)
{
	STATWORD ps;    

	unsigned long start_time, end_time;
	if(sysCallCounterFlag == TRUE)
	{
		start_time = ctr1000;
		sysCallCounters[currpid][SLEEP100_NUM]++;
	}

	if (n < 0  || clkruns==0)
	{
		if(sysCallCounterFlag == TRUE)
		{
			end_time = ctr1000;
			sysCallExecTimes[currpid][SLEEP100_NUM] += end_time - start_time;
		}
	         return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep100(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n*10);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);
	if(sysCallCounterFlag == TRUE)
	{
		end_time = ctr1000;
		sysCallExecTimes[currpid][SLEEP100_NUM] += end_time - start_time;
	}
	return(OK);
}
