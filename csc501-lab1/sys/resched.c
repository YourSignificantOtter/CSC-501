/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

#include "lab1.h"

//#define DEBUG_PRINT

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

		int qiter = q[rdyhead].qnext;
		while(qiter == 0) {
			qiter = q[qiter].qnext;
		}
		int min = MAXINT; //max prio val is 99
		int toRun = MAXINT;
		Bool greater = FALSE; //r is greater than the highest prio
		Bool less = FALSE; //r is less than the lowest prio
		Bool empty = FALSE; //rdy queue is empty

		if(isempty(rdyhead))
		{
			toRun = 0; //run null proc
			empty = TRUE;
		}
		else if(r >= q[q[rdytail].qprev].qkey) //Is random value greater than the highest priority?
		{
			toRun = q[rdytail].qprev;
			greater = TRUE;
		}
		else if(r < q[qiter].qkey)  //Is random value less than the lowest priority?
		{
			do { //Round robin
				toRun = qiter;
				qiter = q[qiter].qnext;
			}while(q[qiter].qkey == q[toRun].qkey);
			less = TRUE;
		}

		qiter = q[rdyhead].qnext; //reset qiter
		while(qiter != rdytail && !greater && !less && !empty) // Find the right proc to run if none of the above
		{
			if(qiter == 0) // null process
			{
				qiter = q[qiter].qnext;
				continue;
			}

			if(q[qiter].qkey >= r)
			{
				if(q[qiter].qkey <= min) //Round robin with the comparison operator
				{
					min = q[qiter].qkey;
					toRun = qiter;
				}
				else
					break; //The queue is ordered so prioorities will only get higher
			}
			qiter = q[qiter].qnext;
		}

		struct pentry *optr = &proctab[currpid];
		struct pentry *nptr = &proctab[toRun];

		#ifdef DEBUG_PRINT
			kprintf("r: %d\n", r);
//			kprintf("min: %d\ttoRun: %d\n", min, toRun);
//			kprintf("currpid: %d\tnew pid: %d\n", currpid, toRun);
			kprintf("Current Process: %s\tCurrent Prio: %d\n", optr->pname, optr->pprio);
			kprintf("New Process: %s\tNew Prio: %d\n", nptr->pname, nptr->pprio);
		#endif

		//Compare the PID we have found with the currently running process
		if(empty)
		{
			if(optr->pstate == PRCURR)
				return OK; //No context switch needed just continue this process 
		}
		else if(greater)
		{
			if(optr->pprio > q[toRun].qkey && optr->pprio >= r)
				return OK; //The current process has higher priority and the randomly generated number is greater than all priorities
		}
		else if(less)
		{
			if(optr->pprio < q[toRun].qkey)
			{
				if(optr->pprio >= r)
					return OK;
			}
		}

		if(optr->pprio >= r && optr->pprio < nptr->pprio)
		{
			return OK; //continue to run the current process
		}
		toRun = dequeue(toRun); //Pop the new process out the queue and insert the current process back in

		if(optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio); //We inserted the process into the queue up above
		}

		nptr->pstate = PRCURR;

		#ifdef	RTCLOCK
			preempt = QUANTUM;		// reset preemption counter	
		#endif

		currpid = toRun; //Set the currpid global to the new pid
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

