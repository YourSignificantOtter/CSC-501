#include <kernel.h>
#include <proc.h>

#include <lock.h>

/*
  ============================================================
	linit - initialize the lock table
  ============================================================
*/
void linit(void)
{
	#ifdef DBG_PRINT
		kprintf("DBG_PRINT: Initializing locktab...\t");
	#endif

	int i = 0;
	//Loop over the lock table
	for(; i < NLOCKS; i++)
	{
		locktab[i].status = FREE;
		locktab[i].owner = -1;
		int j = 0;
		//Loop over the pids
		for(; j < NPROC; j++)
		{
			locktab[i].pid[j] = FALSE;
			locktab[i].prio[j] = 0;
		}
	}

	#ifdef DBG_PRINT
		kprintf("Done!\n");
	#endif
}
