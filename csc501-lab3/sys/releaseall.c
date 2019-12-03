#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include <lock.h>

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
	if(numlk < 0 || numlk > NLOCKS)
	{
		#ifdef DBG_PRINT
			kprintf("DBG_PRINT: Call to releaseall with numlocks %d is out of bounds 0 - %d!\n", numlk, NLOCKS);
		#endif
		return SYSERR;
	}

	#ifdef DBG_PRINT
		kprintf("DBG_PRINT: Releasing %d locks...\t", numlk);
	#endif

	// Disable interrupts
	STATWORD ps;
	disable(ps);

	//Get access to the passed arguments via array

	// Re-enable interrupts
	#ifdef DBG_PRINT
		kprintf("done!\n");
	#endif;
	restore(ps);
	return OK;
}
