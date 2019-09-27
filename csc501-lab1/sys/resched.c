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

int goodnessvals[NPROC];

int calcgoodness(int pid)
{
        if(pid == 0) //Null process has no goodness
                return 0;

        if(proctab[pid].pcounter == 0) //process has consumed all of its quantum, no goodness
                return 0;

        return proctab[pid].pcounter + proctab[pid].pprio; //use OLD pprio
}

int newepochupdate(int pid)
{
	if(proctab[pid].pcounter == proctab[pid].pquantum) //This process didnt run last epoch
		proctab[pid].pquantum = proctab[pid].pnewprio;
	else if(proctab[pid].pcounter == 0) //This process used all its quantum last epoch
		proctab[pid].pquantum = proctab[pid].pnewprio;
	else //This process excuted PART of its quantum last epoch
		proctab[pid].pquantum = (proctab[pid].pcounter / 2) + proctab[pid].pnewprio;

	proctab[pid].pprio = proctab[pid].pnewprio; //update the priorities
	proctab[pid].pcounter = proctab[pid].pquantum; //reset all the pcounters

	return proctab[pid].pcounter + proctab[pid].pprio;
}

void printexpdistscheddebug(int r, struct pentry *optr, struct pentry *nptr)
{
	#ifdef DEBUG_PRINT
		kprintf("r: %d\n", r);
		kprintf("Current Process: %s\tCurrent Prio: %d\n", optr->pname, optr->pprio);
		kprintf("New Process: %s\tNew Prio: %d\n", nptr->pname, nptr->pprio);
	#endif
}

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
		int min = MAXINT; //max prio val is 99
		int toRun = 0;
		struct pentry *optr = &proctab[currpid];

		if(q[rdyhead].qnext == rdytail) //Is the queue empty
		{
			if(optr->pstate == PRCURR)
				return OK; //keep running this process
	
			toRun = 0; //Run the null process
			struct pentry *nptr = &proctab[toRun];		
			currpid = toRun; //Set the currpid global to the new pid
			nptr->pstate = PRCURR;
			printexpdistscheddebug(r, optr, nptr);
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
			#ifdef	RTCLOCK
				preempt = QUANTUM;		// reset preemption counter	
			#endif
			return OK;

		}

		if(r >= lastkey(rdytail)) //Is random value greater than the highest priority?
		{
			toRun = q[rdytail].qprev;
			struct pentry *nptr = &proctab[toRun];		
			//Compare the PID we have found with the currently running process
			if(optr->pprio > q[toRun].qkey && optr->pstate == PRCURR) //The current process is greater than the one at the end of the list
			{
				return OK; //The current process has higher priority and the randomly generated number is greater than all priorities
			}
			if(optr->pstate == PRCURR)
			{
				optr->pstate = PRREADY;
				insert(currpid, rdyhead, optr->pprio); 
			}
			toRun = dequeue(toRun);
			currpid = toRun; //Set the currpid global to the new pid
			nptr->pstate = PRCURR;
			printexpdistscheddebug(r, optr, nptr);
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
			#ifdef	RTCLOCK
				preempt = QUANTUM;		// reset preemption counter	
			#endif
			return OK;
		}

		if(r < firstkey(rdyhead))  //Is random value less than the lowest priority? (that isnt the null proc)
		{
			do { //Round robin
				toRun = qiter;
				qiter = q[qiter].qnext;
			}while(q[qiter].qkey == q[toRun].qkey);
			struct pentry *nptr = &proctab[toRun];		
			if((optr->pprio < q[toRun].qkey && optr->pprio > r) && optr->pstate == PRCURR)
			{
				return OK;
			}
			if(optr->pstate == PRCURR)
			{
				optr->pstate = PRREADY;
				insert(currpid, rdyhead, optr->pprio); //We inserted the process into the queue up above
			}
			toRun = dequeue(toRun);
			currpid = toRun; //Set the currpid global to the new pid
			nptr->pstate = PRCURR;
			printexpdistscheddebug(r, optr, nptr);
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
			#ifdef	RTCLOCK
				preempt = QUANTUM;		// reset preemption counter	
			#endif
			return OK;

		}

		qiter = q[rdyhead].qnext; //reset qiter
		while(qiter != rdytail) // Find the right proc to run if none of the above
		{
			toRun = qiter;
			if(q[qiter].qkey > r)
			{
				if(q[qiter].qkey <= min) //Round robin with the comparison operator
				{
					min = q[qiter].qkey;
				}
				else
				{
					toRun = q[qiter].qprev; //roll toRun back 1
					break; //The queue is ordered so prioorities will only get higher
				}
			}
			qiter = q[qiter].qnext;
		}

		struct pentry *nptr = &proctab[toRun];
		if(optr->pprio > r && optr->pprio < nptr->pprio && optr->pstate == PRCURR)
		{
			return OK; //continue to run the current process
		}

		if(optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio); //We inserted the process into the queue up above
		}

		nptr = &proctab[ (currpid = dequeue(toRun)) ];
		nptr->pstate = PRCURR;
		#ifdef	RTCLOCK
			preempt = QUANTUM;		// reset preemption counter	
		#endif
		printexpdistscheddebug(r, optr, nptr);	
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		// The OLD process returns here when resumed.
		return OK;					
	}
	else if(sched_class == LINUXSCHED)
	{

		//Clear goodnessvals from last run
		int i = 0;
		struct pentry *optr = &proctab[currpid];
		struct pentry *nptr;
		for(; i < NPROC; i++)
			goodnessvals[i] = 0;

		proctab[currpid].pcounter = preempt;

		//Check for new epoch
		int newEpoch = 0;
		for(i = 0; i < NPROC; i++)
		{
			if(proctab[i].pstate == PRCURR || proctab[i].pstate == PRREADY) //Same thing as traversing ready queue, could update this
			{
				newEpoch = calcgoodness(i);
				if(newEpoch > 0)
					break; //No need to keep checking
			}
		}

		int toRun = 0;
		int maxGoodness = 0;
		if(newEpoch == 0)
		{
			#ifdef DEBUG_PRINT
				kprintf("\n\nNew Epoch\n");
			#endif
			//if new epoch get new goodness values
			for(i = 0; i < NPROC; i++)
			{
				if(proctab[i].pstate != PRFREE)
					goodnessvals[i] = newepochupdate(i);

				if(goodnessvals[i] > maxGoodness && (proctab[i].pstate == PRCURR || proctab[i].pstate == PRREADY))
				{
					maxGoodness = goodnessvals[i];
					toRun = i;
				}
			}
		}
		else
		{
			//Find max goodness without calculating new epoch update
			for(i = 0; i < NPROC; i++)
			{
				if(proctab[i].pstate == PRCURR || proctab[i].pstate == PRREADY) //Same thing as traversing ready queue, could update this
				{			
					goodnessvals[i] = calcgoodness(i);
					if(goodnessvals[i] > maxGoodness)
					{
						maxGoodness = goodnessvals[i];
						toRun = i;
					}
				}
			}

		}
		//Context switch
		if(currpid == toRun && toRun != 0)
		{
			return OK;
		}

		if(toRun == 0)
		{
			//Make sure that the ready queue is empty, else we shouldnt be running the nullproc!
			if(isempty(rdyhead))
			{
				//Context switch to the null process
				if(currpid == toRun)
					return OK;
				//Fall through to the context switch below
								
			}
			else
			{
				#ifdef DEBUG_PRINT
					kprintf("Running null proc when we shouldnt!\n");
				#endif
			}
		}

		//Context switch to the next process
		#ifdef DEBUG_PRINT
			kprintf("\nCurrent Process: %s\tCurrent Goodness: %d\tCurrent pcounter: %d\n", proctab[currpid].pname, goodnessvals[currpid], proctab[currpid].pcounter);
			kprintf("Next Process: %s\t Next Goodness: %d\tNext pcounter: %d\n", proctab[toRun].pname, goodnessvals[toRun], proctab[toRun].pcounter);
		#endif

		if(optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}

		toRun = dequeue(toRun);
		nptr = &proctab[toRun];		
		currpid = toRun;
		nptr->pstate = PRCURR;
		#ifdef	RTCLOCK
			preempt = proctab[toRun].pcounter;		// reset preemption counter	
		#endif
	
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
		// The OLD process returns here when resumed.
		return OK;

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

