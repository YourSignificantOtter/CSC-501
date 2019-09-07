/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */
SYSCALL getpid()
{
	unsigned long start_time, end_time;
	if(sysCallCounterFlag == TRUE)
	{
		start_time = ctr1000;
		sysCallCounters[currpid][GETPID_NUM]++;
	}

	if(sysCallCounterFlag == TRUE)
	{
		end_time = ctr1000;
		sysCallExecTimes[currpid][GETPID_NUM] += end_time - start_time;
	}

	return(currpid);
}
