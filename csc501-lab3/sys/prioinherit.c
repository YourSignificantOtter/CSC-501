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
		kprintf("\t\t\tPerforming priority inheritance tasks!\n");
	#endif


	struct pentry *nptr = &proctab[pid]; //The new process that wants the lock
	struct pentry *optr = &proctab[locks[lock].owner]; //The process that has the lock

	//Check if the new process has prio > old process
	if(pinhpprio(nptr) > pinhpprio(optr))
	{
		kprintf("new pointer greater than old\n");
		optr->pinh = pinhpprio(nptr); //Update the pinh field of the curr
		//lock owner to be the new greaer priority value

		//Perform transative priority inheritance
		int l = 0;
		for(; l < NLOCK; l++)
		{
			if(proctab[pid].plocks[l] == TRUE && l != lock)
			{
				struct pentry *lptr = &proctab[locks[l].owner];
				if(pinhpprio(nptr) > pinhpprio(lptr))
				{
					kprintf("Prio inherit recursion?\n");
					lptr->pinh = pinhpprio(nptr);
					prio_inherit(locks[l].owner, l);
				}
			}
		}
	}	

	restore(ps);
	return OK;
}






/*

	if(lk->status == READ)
	{
		//TODO Check multiple processes that might be reading at once
	}
	else if(lk->status == WRITE)
	{
		//Check priority levels
		int lockOwner = 0;
		for(; lockOwner < NPROC; lockOwner++)
		{
			if(lk->currpids[lockOwner] == TRUE) //Thre can only be ONE writer at a time
				break;
		}

		#ifdef DBG_PRINT
			kprintf("\tlockOwner: %d\n", lockOwner);
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

*/
