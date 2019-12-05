#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>

#include <lock.h>

int readyBlockedProcess(int lkId); //Ready a process that is blocked waiting on the released lock

/*
  ============================================================
	releaseall - releases a variable number of locks from the calling process
		numlk - the number of locks to release
		args - the passed arguments, used as an array
		returns OK or SYSERR
  ============================================================
*/
int releaseall(int numlk, long args)
{
	// Disable interrupts
	STATWORD ps;
	disable(ps);

	if(numlk < 0 || numlk > NLOCKS)
	{
		#ifdef DBG_PRINT
			kprintf("DBG_PRINT: Call to releaseall with numlocks %d is out of bounds 0 - %d!\n", numlk, NLOCKS);
		#endif
		restore(ps);
		return SYSERR;
	}

	#ifdef DBG_PRINT
		kprintf("DBG_PRINT: Releasing %d locks...\t", numlk);
	#endif

	#ifdef DBG_PRINT
		kprintf("\n\nDBG_PRINT: Process %d (%s) Is releasing the lock!\n", currpid, proctab[currpid].pname);
	#endif

	//Get access to the passed arguments via array
	struct pentry *pptr = &proctab[currpid];
	unsigned long *a = (unsigned int)(&args);
	Bool badLock = FALSE;
	int currentLockId;
	int i = 0;
	for(; i < numlk; i++)
	{
		currentLockId = a[i];
		//printLock(currentLockId);

		#ifdef DBG_PRINT
			kprintf("narg: %d id: %d ", numlk, currentLockId);
		#endif

		lock_t *lk = &locktab[currentLockId];
		if(lk->owner != currpid)
		{
			if(lk->status == READ && lk->accType[currpid] == READ)
			{
				//release the lock from a simultaneous reader, just remove it from the lock queue
				lk->pid[currpid] 	= FALSE;
				lk->prio[currpid] 	= 0;
				lk->accType[currpid]	= FREE;
				lk->timeStamp[currpid] 	= 0;

				proctab[currpid].plocks[currentLockId] = FALSE;
			}
			else
			{
				//Otherwise a process tried to release a lock it should not have owned
				#ifdef DBG_PRINT
					kprintf("Failed! lock %d is not owned by process %d(%s)!\n", currentLockId, currpid, proctab[currpid].pname);
				#endif
				badLock = TRUE;
			}
		}
		else
		{
			//release the lock
			lk->status 		= FREE;
			lk->owner 		= -1;
			lk->pid[currpid] 	= FALSE;
			lk->prio[currpid] 	= 0;
			lk->accType[currpid] 	= FREE;
			lk->timeStamp[currpid]	= 0;

			//Clean up the proctab information
			proctab[currpid].plocks[currentLockId] = FALSE;			

			//Check the lock queue to see who is next in line to get the lock
			if(readyBlockedProcess(currentLockId) == SYSERR)
				badLock = TRUE;

			proctab[currpid].pinh = 0;
			priorityInheritance(currentLockId);

		}
	}

/*
	//Clear pinh
	proctab[currpid].pinh = 0;
	//Check if we still own any locks, if so perform prio inheritance on them
	i = 0;
	for(; i < NLOCKS; i++)
	{
		if(proctab[currpid].plocks[i] == TRUE)
		{
			if(locktab[i].owner == currpid)
			{
				priorityInheritance(i);
			}
		}
	}
*/
	//Check if we need to return syserr
	if(badLock == TRUE)
	{
		#ifdef DBG_PRINT
			kprintf("DBG_PRINT: Release all finished with errors\n");
		#endif
		restore(ps);
		return SYSERR;
	}
	else
	{
		#ifdef DBG_PRINT
			kprintf("done!\n");
		#endif;
		restore(ps);
		return OK;
	}	

	#ifdef DBG_PRINT
		kprintf("Dont know how this happened.........\n");
	#endif

	restore(ps);
	return OK;
}

/*
  ============================================================
	readyBlockedProcess - readies the process with the highest LOCK priority in a locks queue
			      after the lock has been released by its previous owner
		lkId - The lock ID of the lock which was released
		returns OK or SYSERR;
  ============================================================
*/
int readyBlockedProcess(int lkId)
{
	if(isbadlock(lkId))
		return SYSERR;

	//Check for the highest LOCK priority process in the locks queue
	lock_t *lk = &locktab[lkId];
	int maxPrioPid = -1;
	int i = 0;
	int maxPrio = MININT;
	for(; i < NPROC; i++)
	{
		if(i == currpid)
			continue;

		if(lk->pid[i] == TRUE)
		{
			if(lk->prio[i] > maxPrio)
			{
				maxPrio = lk->prio[i];
				maxPrioPid = i;
			}
			else if(lk->prio[i] == maxPrio) //In a tie check the time in queue
			{
				//In the case of equal waiting priorities, the lock will be given to the 
				//process that has the longest waiting time (in milliseconds) on the lock. 
				//If the waiting priorities are equal and the waiting time difference is 
				//within 1 second, writers should be given preference to acquire the 
				//lock over readers.

				unsigned long currTime = ctr1000;
				unsigned long waitTimePID1 = lk->timeStamp[maxPrioPid] - currTime;
				unsigned long waitTimePID2 = lk->timeStamp[i] - currTime;

				//check if we are within 1 second of each other
				if((waitTimePID2 >= (waitTimePID1 - 1000)) && (waitTimePID2 <= (waitTimePID1 + 1000)))
				{
					//if one is a writer use that one, 
					//other wise use the one with longer wait time
					if(lk->accType[maxPrioPid] == WRITE && lk->accType[i] == READ)
						maxPrioPid = maxPrioPid;
					else if(lk->accType[maxPrioPid] == READ && lk->accType[i] == WRITE)
						maxPrioPid = i;
					else
					{
						if(waitTimePID1 > waitTimePID2)
							maxPrioPid = maxPrioPid;
						else
							maxPrioPid = i;
					}							
				}
				else //Just take the longer wait time
				{
					if(waitTimePID1 > waitTimePID2)
						maxPrioPid = maxPrioPid;
					else
						maxPrioPid = i;
				}
			}
		}
	}


	if(isbadpid(maxPrioPid))
	{
		//This is the only process that is in the queue, no one else wants it
		return OK;
	}

	struct pentry *pptr = &proctab[maxPrioPid];
	//Check if the new owner is a writer, if so we need to remove this locks readers from ready queue
	if(lk->accType[maxPrioPid] == WRITE)
	{
		int i = 0;
		for(; i < NPROC; i++)
		{
			if(lk->pid[i] == TRUE)
			{
				//Check if that process is currently ready
				if(proctab[i].pstate == PRREADY)
				{
					//Remove it from the ready queue
					kprintf("Remove(%d)\n", i);
					remove(i, rdyhead);
				}
				proctab[i].plockid = lkId;
				proctab[i].pstate = PRWAIT;
				proctab[i].pwaitret = TRUE;
			}
		}
	}

	//Check if the process is waiting on this lock
	if(pptr->pstate == PRWAIT && pptr->plockid == lkId)
	{
		pptr->plockid = -1;
		ready(maxPrioPid, RESCHNO);
	}


	pptr->plocks[lkId] = TRUE;
	
	//Set the new PID to be the lock owner
	lk->status = lk->accType[maxPrioPid];
	lk->owner = maxPrioPid;
	return OK;
}
