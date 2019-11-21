/* wakeup.c - wakeup */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>

/*------------------------------------------------------------------------
 * wakeup  --  called by clock interrupt dispatcher to awaken processes
 *------------------------------------------------------------------------
 */
INTPROC	wakeup()
{
        while (nonempty(clockq) && firstkey(clockq) <= 0)
	{ //Check if the process is waiting on a lock
	  //If it is waiting on the lock check if it is at the head of the lock queue
		int i = 0;
		int pid = getfirst(clockq);
		Bool rdy = FALSE;
		struct pentry *pptr = &proctab[pid];
		for(; i < NLOCK; i++)
		{
			if(locks[i].head->pid == pid)
			{
				rdy = TRUE;
				break;
			}
		}

		if(rdy == TRUE)
		{
			proctab[pid].pstate = PRWAIT;
			proctab[pid].pwaitret = OK;
		}
		else
		{
			ready(pid, RESCHNO);
		}
	}
	if ((slnempty = nonempty(clockq)))
		sltop = & q[q[clockq].qnext].qkey;
	resched();
        return(OK);
}
