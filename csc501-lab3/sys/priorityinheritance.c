#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include <lock.h>

/*
  ============================================================
	priorityInheritance - performs inheritance on the passed processes
		accessor - the PID that wants to gain the lock
		owner - the PID that currently owns the lock
		returns OK or SYSERR
  ============================================================
*/
//int priorityInheritance(int accessor, int owner)
int priorityInheritance(int lkId)
{
	#ifdef DBG_PRINT
		kprintf("Performing priority inheritance...\t");
	#endif
/*
	if(isbadpid(accessor) || isbadpid(owner))
	{
		#ifdef DBG_PRINT
			kprintf("Failed! One of the passed pids %d %d is out of bounds 0 - %d!\n", accessor, owner, NPROC);
		#endif
		return SYSERR;
	}
*/

	if(isbadlock(lkId))
	{
		#ifdef DBG_PRINT
			kprintf("Failed! The passed lock id %d is out of bounds 0 - %d!\n", lkId, NLOCKS);
		#endif
		return SYSERR;
	}

	//Disable Interrupts
	STATWORD ps;
	disable(ps);

	//Prio(p) = max (Prio(p_i)),    for all processes p_i waiting on any of the locks held by process p.
	lock_t *lk = &locktab[lkId];

	int i = 0;
	int maxPrio = MININT;
	//Find the max prio in the lock queue
	for(; i < NPROC; i++)
	{
		if(lk->pid[i] == TRUE)
		{
			if(ppriopinh(i) > maxPrio)
				maxPrio = ppriopinh(i);
		}
	}
	
	//Set the lock owners inherited priority to the new priority
	if(ppriopinh(lk->owner) < maxPrio)
	{
		proctab[lk->owner].pinh = maxPrio;
		//Perform transative priority inheritance
		//Check each lock that this process is in the queue for
		int j = 0;
		for(; j < NLOCKS; j++)
		{
			if(proctab[lk->owner].plocks[j] == TRUE)
				priorityInheritance(j);
		}		
	}

	//Re-enable interrupts
	#ifdef DBG_PRINT
		kprintf("done!\n");
	#endif
	restore(ps);
	return OK;
}

/*
	//Set the owner's priority to the acessor's priority
	proctab[owner].pinh = ppriopinh(accessor);

	//Handle transitive properties
	int i = 0;
	for(; i < NLOCKS; i++)
	{
		if(proctab[owner].plocks[i] == TRUE)
		{
			
		}
	}

	//Re-enable interrupts
	#ifdef DBG_PRINT
		kprintf("done!\n");
	#endif
	restore(ps);
	return OK;
}
*/
