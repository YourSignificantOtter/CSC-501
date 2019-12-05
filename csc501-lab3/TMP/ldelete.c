#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include <lock.h>

/*
  ============================================================
	ldelete - delete a lock, user must own the lock to call this
		lkId - ID of the lock the user wants to delete
		returns OK or SYSERR
  ============================================================
*/
int ldelete(int lkId)
{
	#ifdef DBG_PRINT
		kprintf("DBG_PRINT: Deleting lock...\t");
	#endif

	if(isbadlock(lkId))
	{
		#ifdef DBG_PRINT
			kprintf("Failed! Call to ldelete with lock ID %d is out of bounds 0 - %d!\n", lkId, NLOCKS);
		#endif
	}

	//Clear the lock information
	lock_t *lk = &locktab[lkId];
	lk->status = FREE;
	lk->owner = -1;
	int i = 0;
	for(; i < NPROC; i++)
	{
		lk->pid[i]		= FALSE;
		lk->prio[i]		= 0;
		lk->accType[i]		= FREE;
		lk->timeStamp[i]	= 0;
	}

	i = 0;
	for(; i < NPROC; i++)
	{
		//Check if any processes were waiting on this lock
		if(proctab[i].plockid == lkId)
		{
			#ifdef DBG_PRINT
				kprintf("DBG_PRINT: Process %d (%s) (%d) was waiting on deleted lock %d\n", i, proctab[i].pname, proctab[i].plockid, lkId);
			#endif

			proctab[i].plockid = DELETED; //Inform the process that the lock was deleted
			ready(i, rdyhead);
		}
		
		if(proctab[i].plocks[lkId] == TRUE)
		{
			#ifdef DBG_PRINT
				kprintf("DBG_PRINT: Process %d (%s) owned the deleted lock %d\n", i, proctab[i].pname, lkId);
			#endif
			proctab[i].plocks[lkId] = FALSE;
		}
	}

	#ifdef DBG_PRINT
		kprintf("Done!\n");
	#endif
	return OK;
}
