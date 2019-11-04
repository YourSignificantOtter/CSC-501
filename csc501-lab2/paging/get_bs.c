#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

	STATWORD ps;
	disable(ps);
	/* requests a new mapping of npages with ID map_id */
	//Check early termination values
	if(npages <= 0 || npages > NPGBS)
	{
		#ifdef DBG_PRINT
			kprintf("Cannot get_bs with %d pages!\n", npages);
		#endif
		restore(ps);
		return SYSERR;
	}
	if(bs_id < 0 || bs_id > NBS)
	{
		#ifdef DBG_PRINT
			kprintf("Backing store ID: %d is out of range 0 - 7!\n", bs_id);
		#endif
		restore(ps);
		return SYSERR;
	}

	bs_map_t *bs = &bsm_tab[bs_id];

	if(bs->bs_private != BSM_PUBLIC)
	{
		#ifdef DBG_PRINT
			kprintf("Backing store ID: %d is not public!\n", bs_id);
		#endif
		restore(ps);
		return SYSERR;
	}

	int retVal = npages;

	if(bs->bs_status == BSM_UNMAPPED)
	{
		#ifdef DBG_PRINT
			kprintf("Requested Backing store is free to be mapped\n");
		#endif
		//This backing store is not mapped to anything return the passed number of npages
		//Do we need bsm_map here?
		bs->bs_status = BSM_MAPPED;
		retVal = npages;
	}
	else
	{
		#ifdef DBG_PRINT
			kprintf("Requested Backing store is already publically mapped\n");
		#endif
		//The backing store is already mapped and is already public return npages of that backing store
		retVal = bs->bs_npages;
	}
	
	restore(ps);
	return retVal;
}


