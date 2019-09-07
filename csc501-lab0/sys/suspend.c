/* suspend.c - suspend */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

#include "lab0.h"

/*------------------------------------------------------------------------
 *  suspend  --  suspend a process, placing it in hibernation
 *------------------------------------------------------------------------
 */
SYSCALL	suspend(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority returned		*/

	unsigned long start_time, end_time;
	if(sysCallCounterFlag == TRUE)
	{
		start_time = ctr1000;
		sysCallCounters[currpid][SUSPEND_NUM]++;
	}

	disable(ps);
	if (isbadpid(pid) || pid==NULLPROC ||
	 ((pptr= &proctab[pid])->pstate!=PRCURR && pptr->pstate!=PRREADY)) {
		restore(ps);
		if(sysCallCounterFlag == TRUE)
		{
			end_time = ctr1000;
			sysCallExecTimes[currpid][SUSPEND_NUM] += end_time - start_time;
		}
		return(SYSERR);
	}
	if (pptr->pstate == PRREADY) {
		pptr->pstate = PRSUSP;
		dequeue(pid);
	}
	else {
		pptr->pstate = PRSUSP;
		resched();
	}
	prio = pptr->pprio;
	restore(ps);
	if(sysCallCounterFlag == TRUE)
	{
		end_time = ctr1000;
		sysCallExecTimes[currpid][SUSPEND_NUM] += end_time - start_time;
	}
	return(prio);
}
