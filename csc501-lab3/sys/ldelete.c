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


	#ifdef DBG_PRINT
		kprintf("Done!\n");
	#endif
	return OK;
}
