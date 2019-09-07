/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>

#include "lab0.h"

/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
SYSCALL	setdev(int pid, int dev1, int dev2)
{
	short	*nxtdev;

	if(sysCallCounterFlag == TRUE)
		sysCallCounters[currpid][SETDEV_NUM]++;

	if (isbadpid(pid))
		return(SYSERR);
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;
	return(OK);
}
