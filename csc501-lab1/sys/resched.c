/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

#include "lab1.h"

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	int sched_class = getschedclass();
	if(sched_class == EXPDISTSCHED)
	{
		double tmp = expdev(LAMBDA); //Randomly generated number following exponential distribution
		int r = (int)tmp; //Cast to int because pprio is int
		if(r < 0)
			r = r * -1; //get the abs value of the expdev return
		int min = 100; //max prio val is 99
		int qiter = q[rdyhead].qnext;
		int toRun = MAXINT;

		while(qiter != rdytail)
		{
			if(q[qiter].qkey >= r)
			{
				if(q[qiter].qkey < min)
				{
					min = q[qiter].qkey;
					toRun = qiter;
				}
			}
			qiter = q[qiter].qnext;
		}

		kprintf("min: %d\ttoRun: %d\n", min, toRun);

		struct pentry *optr = &proctab[currpid];
		struct pentry *nptr;

		if(toRun == MAXINT) //There exists no process with prio greater than r, switch to the process with the highest prio
		{
			toRun = getfirst(rdyhead);//First item in the queue
			if(toRun == EMPTY) //No processes are wiating to run, run the nullprocess
				toRun = 0;
		}

		if(toRun == currpid) //No need for context switch, re-insert the current pid into the ready queue
		{
			insert(currpid, rdyhead, optr->pprio);
			return OK;
		}
		else
		{
			toRun = dequeue(toRun); //Pop the new process out the queue and insert the current process back in
			insert(currpid, rdyhead, optr->pprio);
		}

		nptr = &proctab[toRun]; //New process to run		
		kprintf("currpid: %d\tnew pid: %d\n", currpid, toRun);

		//Force a context switch to the new process
		kprintf("Current Process: %s\n", optr->pname);
		kprintf("New Process: %s\n", nptr->pname);
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

		// The OLD process returns here when resumed.
		return OK;					
	}
	else if(sched_class == LINUXSCHED)
	{
		;
	}
	else
	{
		register struct	pentry	*optr;	// pointer to old process entry 
		register struct	pentry	*nptr;	// pointer to new process entry 

		// no switch needed if current process priority higher than next

		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		   (lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
	
		// force context switch 

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		// remove highest priority process at end of ready list 

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		// mark it currently running	
		#ifdef	RTCLOCK
			preempt = QUANTUM;		// reset preemption counter	
		#endif
	
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
		// The OLD process returns here when resumed.
		return OK;
	}
}

