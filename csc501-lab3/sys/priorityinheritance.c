#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include <lock.h>

/*
  ============================================================
	priorityInheritance - performs inheritance on the passed lock
		lkId - ID of the lock to perform inheritance on
		returns OK or SYSERR
  ============================================================
*/
int priorityInheritance(int lkId)
{
	#ifdef DBG_PRINT
		kprintf("Performing priority inheritance...\t");
	#endif

	if(isbadlock(lkId))
	{
		#ifdef DBG_PRINT
			kprintf("Failed! The passed lock id %d is out of bounds 0 - %d!\n", lkId, NLOCKS);
		#endif
		return SYSERR;
	}

	if(isbadpid(locktab[lkId].owner))
	{
		#ifdef DBG_PRINT
			kprintf("Failed! The passed lock owner is not a valid pid!\n");
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
		if(i == lk->owner)
			continue;

		if(lk->pid[i] == TRUE)
		{
			if(ppriopinh(i) > maxPrio)
			{
				maxPrio = ppriopinh(i);
			}
		}
	}

	//If there is no priority to inherit set the inherited priority of the lock owner to 0
	if(maxPrio == MININT)
	{
		proctab[lk->owner].pinh = 0;
	}
	//Set the lock owners inherited priority to the new priority
	if(proctab[lk->owner].pprio < maxPrio)
	{
		proctab[lk->owner].pinh = maxPrio;
		//Perform transative priority inheritance
		//Check each lock that this process is in the queue for
		int j = 0;
		for(; j < NLOCKS; j++)
		{
			if(j == lkId)
				continue;

			if(locktab[j].pid[lk->owner] == TRUE)
				priorityInheritance(j);
			/*
			if(proctab[lk->owner].plocks[j] == TRUE)
				priorityInheritance(j);
			*/
		}		
	}

	//Re-enable interrupts
	#ifdef DBG_PRINT
		kprintf("done!\n");
	#endif
	restore(ps);
	return OK;
}
