/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include "lab0.h"

/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */
SYSCALL	setnok(int nok, int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	unsigned long start_time, end_time;
	if(sysCallCounterFlag == TRUE);
	{
		start_time = ctr1000;
		sysCallCounters[currpid][SETNOK_NUM]++;
	}

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
		if(sysCallCounterFlag == TRUE)
		{
			end_time = ctr1000;
			sysCallExecTimes[currpid][SETNOK_NUM] += end_time - start_time;
		}
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);

	if(sysCallCounterFlag == TRUE)
	{
		end_time = ctr1000;
		sysCallExecTimes[currpid][SETNOK_NUM] += end_time - start_time;
	}

	return(OK);
}
