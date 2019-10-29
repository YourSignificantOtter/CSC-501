#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

	/* requests a new mapping of npages with ID map_id */
	//Check early termination values
	if(npages <= 0 || npages > 256)
	{
		#ifdef DBG_PRINT
			kprintf("Cannot get_bs with %d pages!\n", npages);
		#endif
		return SYSERR;
	}
	if(bs_id < 0 || bs_id > 7)
	{
		#ifdef DBG_PRINT
			kprintf("Backing store ID: %d is out of range 0 - 7!\n", bs_id);
		#endif
		return SYSERR;
	}

	return OK;
}


