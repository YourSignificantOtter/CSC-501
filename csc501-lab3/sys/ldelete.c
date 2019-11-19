#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

/* -------------------------------------------------------
 * ldelete - delete a lock
 * -------------------------------------------------------
 */
int ldelete(int lockdescriptor)
{
	#ifdef DBG_PRINT
		kprintf("Deleting lock: %d\n");
	#endif

	if(isbadlock(lockdescriptor))
	{
		#ifdef DBG_PRINT
			kprintf("Bad lock descriptor passed, cannot be deleted!\n");
		#endif
		return SYSERR;
	}

	locks[lockdescriptor].status = DELETED;
	kprintf("TODO: Flush the queue in the lock so that other processes know its deleted and dont try to use it!\n");

	return SYSERR;
}
