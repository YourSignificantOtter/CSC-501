/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

#include "lab0.h"

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	unsigned long start_time, end_time;
	if(sysCallCounterFlag == TRUE)
	{
		sysCallCounters[currpid][CHPRIO_NUM]++;
		start_time = ctr1000;
	}

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		if(sysCallCounterFlag == TRUE)
		{
			end_time = ctr1000;
			sysCallExecTimes[currpid][CHPRIO_NUM] += end_time - start_time;
		}
		return(SYSERR);
	}
	pptr->pprio = newprio;
	restore(ps);
	if(sysCallCounterFlag == TRUE)
	{
		end_time = ctr1000;
		sysCallExecTimes[currpid][CHPRIO_NUM] += end_time - start_time;
	}
	return(newprio);
}
