#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

/* -------------------------------------------------------
 * prio_inherit - perform tasks for priority inheritance
 *	pid - the process that is attempting to take the lock
 *	lock - the lock ID
 * returns OK or SYSERR
 * -------------------------------------------------------
 */
int prio_inherit(int pid, int lock)
{
	if(isbadpid(pid))
	{
		#ifdef DBG_PRINT
			kprintf("\tPID passed to prio_inherit is not valid!\n");
		#endif
		return SYSERR;
	}

	if(isbadlock(lock))
	{
		#ifdef DBG_PRINT
			kprintf("\tLock passed to prio_inherit is not valid!\n");
		#endif
		return SYSERR;
	}

	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("\n\n\tPerforming priority inheritance tasks!\n");
	#endif

	struct pentry *pptr = &proctab[currpid];
	lock_t *lk = &locks[lock];

	if(lk->status == READ)
	{
		//TODO Check multiple processes that might be reading at once
	}
	else if(lk->status == WRITE)
	{
		//Check priority levels
		int lockOwner = 0;
		int temp = lk->currpids;
		while(temp)
		{
			lockOwner++;
			temp = temp >> 1;
		}

		#ifdef DBG_PRINT
			kprintf("lk->currpids: 0x%08X\tlockOwner: %d\n", lk->currpids, lockOwner);
		#endif

		int lockOwnerPrio = 0;
		int currPrio = 0;

		if(proctab[lockOwner].pinh != 0)
			lockOwnerPrio = proctab[lockOwner].pinh;
		else
			lockOwnerPrio = proctab[lockOwner].pprio;

		if(pptr->pinh != 0)
			currPrio = pptr->pinh;
		else
			currPrio = pptr->pprio;

		if(lockOwnerPrio >= currPrio)
		{
			#ifdef DBG_PRINT
				kprintf("\tLock owner priority greater than or equal to new pid priority, no changes required\n");
			#endif
			return OK;
		}

		//The current priority is higher than the lock owners, need to update its pinh
		//Also need to perform transitive changes!
		proctab[lockOwner].pinh = currPrio;

		if(proctab[lockOwner].lockid != -1)
		{
			//Transative stuff
			#ifdef DBG_PRINT
				kprintf("Do transative things!\n");
			#endif
		}
		
	}
	else
	{
		#ifdef DBG_PRINT
			kprintf("Something is wrong here\n");
		#endif
		return SYSERR;
	}	


	restore(ps);
	return OK;
}
