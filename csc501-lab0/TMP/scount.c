/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>

#include <proc.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
SYSCALL scount(int sem)
{
	extern	struct	sentry	semaph[];

	if(sysCallCounterFlag == TRUE)
		sysCallCounters[currpid][SCOUNT_NUM]++;

	if (isbadsem(sem) || semaph[sem].sstate==SFREE)
		return(SYSERR);
	return(semaph[sem].semcnt);
}
