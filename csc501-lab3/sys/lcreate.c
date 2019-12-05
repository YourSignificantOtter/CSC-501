#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include <lock.h>

/*
  ============================================================
	lcreate - Create an initialized / deleted lock for a user
	returns the lock ID of the created lock OR syserr if no locks available
  ============================================================
*/
int lcreate(void)
{
	#ifdef DBG_PRINT
		kprintf("DBG_PRINT: creating lock...\t");
	#endif

	int i = 0;
	//Loop over lock table
	for(; i < NLOCKS; i++)
	{
		if(locktab[i].status == FREE)
		{
			#ifdef DBG_PRINT
				kprintf("Done! Lock %d created!\n", i);
			#endif
			return i;
		}
	}

	#ifdef DBG_PRINT
		kprintf("Failed to create lock! No locks are available!\n");
	#endif

	return SYSERR;
}
