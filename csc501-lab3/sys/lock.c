#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include <lock.h>

int blockProcess(int lkId, int lkPrio); //Block the process to wait on the lock

/*
  ============================================================
	lock - user attempting to gain a lock, returns OK or blocks the calling process
		lkId - the ID of the lock the user wants to gain
		accessType - The type of access the user wants, must be READ or WRITE
		lkPrio - The priority of the process in the lock queue NOT its scheduling priority
  ============================================================
*/
int lock(int lkId, int accessType, int lkPrio)
{
	if(isbadlock(lkId))
	{
		#ifdef DBG_PRINT
			kprintf("DBG_PRINT: Call to lock with lock id %d failed! Lock id is out of bounds 0 - %d!\n", lkId, NLOCKS);
		#endif
		return SYSERR;
	}

	if(accessType != READ && accessType != WRITE)
	{
		#ifdef DBG_PRINT
			kprintf("DBG_PRINT: Call to lock with access type %d failed! Value must be READ(%d) or WRITE(%d)!\n", accessType, READ, WRITE);
		#endif
		return SYSERR;
	}

	//Disable interrupts
	STATWORD ps;
	disable(ps);

	//Get the lock from the lock table
	lock_t *lk = &locktab[lkId];

	//Check if the lock is free
	if(lk->status == FREE)
	{
		#ifdef DBG_PRINT
			kprintf("DBG_PRINT: Lock %d was free, acquiring lock!\n", lkId);
		#endif

		//Set up the lock values
		lk->status		= accessType;
		lk->owner 		= currpid;
		lk->maxPrioPid 		= currpid;
		lk->pid[currpid] 	= TRUE;
		lk->prio[currpid] 	= lkPrio;
		lk->accType[currpid] 	= accessType;		

		//Set up the proctab values
		proctab[currpid].plocks[lkId] = TRUE;		
	}
	//Check if we can perform a simulatenous read
	else if(lk->status == READ && accessType == READ)
	{
		//If the requesting process has specified the lock type as READ and has sufficiently high priority 
		//(not less than the highest priority writer process waiting for the lock), 
		//it acquires the lock, else not.

		// Find the highest write priority in the lock queue
		int i;
		int maxWritePrio = MININT;
		for(; i < NPROC; i++)
		{
			if(lk->owner == i)
				continue; //Skip if we are looking at the owner (who is a reader)

			if(lk->pid[i] == TRUE)
			{
				#ifdef DBG_PRINT
					kprintf("DBG_PRINT: pid i = %d, %s, %d, %d\n", i, lk->accType[i] == WRITE ? "WRITE" : "READ ", lk->prio[i], maxWritePrio);
				#endif

				if(lk->accType[i] == WRITE && lk->prio[i] > maxWritePrio)
					maxWritePrio = lk->prio[i];
			}
		}

		if(lkPrio >= maxWritePrio)
		{

			#ifdef DBG_PRINT
				kprintf("DBG_PRINT: Simultaneous read!\n");
			#endif

			//Add the process into the lock queue so we know that its there, but dont block the process
			lk->pid[currpid] = TRUE;
			lk->prio[currpid] = lkPrio;
			//No need to perform priority inheritance here because this process is NOT blocked
		}
		else
		{
			#ifdef DBG_PRINT
				kprintf("DBG_PRINT: Cannot simultaneously read, there is a WRITE process with higher priority in the queue!\n");
			#endif
			blockProcess(lkId, lkPrio);
			//Block by rescheduling
			resched();
			//Upon return the process will have access to the lock!
		}
	}
	//If not we add the process into the "queue"
	else
	{
		blockProcess(lkId, lkPrio);
		//Block by rescheduling
		resched();
		//Upon return the process will have access to the lock!
	}

	restore(ps);
	return OK;
}

/*
  ============================================================
	blockProcess - blocks a process that is trying to gain a lock
		lkId - the lock id that the process is trying to gain
		lkPrio - the priority that the process wants to place into the lock queue
	returns OK or SYSERR
  ============================================================
*/
int blockProcess(int lkId, int lkPrio)
{
	#ifdef DBG_PRINT
		kprintf("DBG_PRINT: Add process %d(%s) to lock queue and block!\n", currpid, proctab[currpid].pname);
	#endif

	lock_t *lk = &locktab[lkId];

	//Enter the new process and its priority to the lock queue
	lk->pid[currpid] = TRUE;
	lk->prio[currpid] = lkPrio;
	lk->accType[currpid] = WRITE;

	//Block the current process
	proctab[currpid].plockid = lkId;
	proctab[currpid].pstate = PRWAIT; //Process is in waiting state
	proctab[currpid].pwaitret = TRUE;

	//Perform priority inheritance
//	return(priorityInheritance(currpid, lk->owner));
	return(priorityInheritance(lkId));
}
