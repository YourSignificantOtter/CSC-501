#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

/* -------------------------------------------------------
 * lreleaseall - simultaneous release of numlocks
 * variadic function that takes inputs as integers on the 
 * stack. ints are lock ids
 * -------------------------------------------------------
 */
int releaseall(int numlocks, long args)
{
	if(numlocks <= 0 || numlocks > NLOCK)
	{
		kprintf("Call to releaseall with numlocks %d is not valid! range is 1 - %d!\n", numlocks, NLOCK);
		return SYSERR;
	}

	STATWORD ps;
	disable(ps);

	unsigned long *a; // points to list of args
	struct pentry *pptr = &proctab[currpid];
	lock_t *lk;
	int currentLock = -1;

	#ifdef DBG_PRINT
		kprintf("\tReleasing %d locks from process %s (PID %d)\n", numlocks, pptr->pname, currpid);
	#endif

	//Get all the lock ids
	a = (unsigned int)(&args) + (numlocks - 1); //Taken from sys/create
	for(numlocks; numlocks > 0; numlocks--)
	{
		currentLock = *a--; //Get the lock ID and move the pointer for the next loop
		#ifdef DBG_PRINT
			kprintf("\treleaseall narg: %d *a: %d\n", numlocks, currentLock);
		#endif

		//Check if its a good lock id
		if(isbadlock(currentLock))
		{
			kprintf("Passed a bad lock ID in releaseall!\n");
			return SYSERR;
		}

		lk = &locks[currentLock];

		//Check if the process actually controls the locks its attempting to release
		if(lk->currpids[currpid] == FALSE)
		{
			kprintf("Process %s does not control lock %d that it was attempting to release!\n", pptr->pname, currentLock);
			return SYSERR;
		}

		//release the lock
		if(lk->status == WRITE) //Can only be one writer at a time so we can safely release this
		{
			lk->status = FREE; //Lock is not used atm
			lk->currpids[currpid] = FALSE; //remove this pid from the current users
			q_dequeue(pinhpprio(pptr), WRITE, currpid, currentLock);
			pptr->plocks[currentLock] = FALSE;
		}
		else if(lk->status == READ) //Might be multiple readers
		{
			lk->status = FREE; //Lock is not used atm
			lk->currpids[currpid] = FALSE; //remove this pid from the current users
			q_dequeue(pinhpprio(pptr), WRITE, currpid, currentLock);
			pptr->plocks[currentLock] = FALSE;

/*
			int i = 0;
			Bool multipleReaders = FALSE;
			for(; i < NLOCK; i++)
			{
				if(locks[currentLock].currpids[i] == TRUE && i != currpid)
				{
					multipleReaders = TRUE;//Multiple readers
					break;
				}
			}

			if(multipleReaders == TRUE)
			{
				//release just this pid from the lock
				lk->currpids[currpid] = FALSE;
				
			}
			else
			{

			}
*/
		}
		
	}	

	restore(ps);
	return OK;
}
