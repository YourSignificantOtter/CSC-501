/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	STATWORD ps;
	disable(ps);

	if(source < 0 || source >= NBS)
	{
		#ifdef DBG_PRINT
			kprintf("xmmap source is out of range 0 - 7\n");
		#endif
		restore(ps);
		return SYSERR;
	}

	if(npages <= 0 || npages >= NPGBS)
	{
		#ifdef DBG_PRINT
			kprintf("xmmap npages is out of range 0 - 1k\n");
		#endif;
		restore(ps);
		return SYSERR;
	}

	if(virtpage < VIRTMEMSTART)
	{
		#ifdef DBG_PRINT
			kprintf("virtpage %d too low, must be >= 4096\n", virtpage);
		#endif
		restore(ps);
		return SYSERR;
	}

	//the BSM must be mapped already
	bs_map_t *bs = &bsm_tab[source];
	if(bs->bs_status == BSM_UNMAPPED)
	{
		#ifdef DBG_PRINT
			kprintf("Backing Store %d must be mapped before call to xmmap\n", source);
		#endif
		restore(ps);
		return SYSERR;
	}

	//Attempt to create mapping
	if(bsm_map(currpid, virtpage, source, npages) == SYSERR)
	{
		#ifdef DBG_PRINT
			kprintf("xmmap failed\n");
		#endif
		restore(ps);
		return SYSERR;
	}

	#ifdef DBG_PRINT
		kprintf("xmmap passed, currpid: %d virtpage: %08X, bs_id: %d, npages: %d\n", currpid, virtpage, source, npages);
	#endif

	restore(ps);
	return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
	STATWORD ps;
	disable(ps);

	//Check if the virtpage is actually mapped in any backing stores
	int i = 0;
	int store = 0, page = 0;
	for(;i < NBS; i++)
	{
		
		if(bsm_lookup(currpid, virtpage * NBPG, &store, &page) == SYSERR)
		{
			continue; //these arent the droids you are looking for
		}
		else
		{
			//This is the bsm we want to unmap, remove this from the backing store
			bsm_unmap(currpid, virtpage, i);
			restore(ps);
			return OK;		
		}
		
	}

	#ifdef DBG_PRINT
		kprintf("Call to xmunmap(%d) failed. No backing store exists mapping the process %s to the given virtual page!\n", virtpage, proctab[currpid].pname);
	#endif

	restore(ps);
	return SYSERR;
}
