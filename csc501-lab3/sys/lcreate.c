#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

/* -------------------------------------------------------
 * lcreate - create a lock and return the locks ID
 * -------------------------------------------------------
 */
int lcreate(void)
{
	#ifdef DBG_PRINT
		kprintf("Creating a Lock!\n");
	#endif

	int i = 0;
	for(; i < NLOCK; i++)
	{
		if(locks[i].status == DELETED || locks[i].status == INIT)
		{
			#ifdef DBG_PRINT
				kprintf("Lock %d is either deleted or currently unused, preparing this lock!\n", i);
			#endif
			locks[i].status = FREE; //The lock is created but has not yet had "lock" called on it
			return i;
		}
	}

	#ifdef DBG_PRINT
		kprintf("No locks were available to return!\n");
	#endif

	return SYSERR;
}
