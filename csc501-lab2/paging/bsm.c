/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

bs_map_t bsm_tab[NBS];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD ps;
	disable(ps); //disable interrupts

	#ifdef DBG_PRINT
		kprintf("Initiaizlize backing stores\n");
	#endif

	int i = 0;
	for(i; i < NBS; i++)
	{
		bsm_tab[i].bs_private	= BSM_PUBLIC;
		bsm_tab[i].bs_status	= BSM_UNMAPPED;
		int j = 0;
		for(; j < NPROC; j++)
			bsm_tab[i].bs_pid[j]	= FALSE;

		bsm_tab[i].bs_vpno	= 0;
		bsm_tab[i].bs_npages	= BSM_UNMAPPED;
		bsm_tab[i].bs_sem	= NULL;

		#ifdef DBG_PRINT
			kprintf("bsm_tab[%d]\n", i);
			print_bsm(bsm_tab[i]);
			kprintf("\n");
		#endif
	}

	#ifdef DBG_PRINT
		kprintf("Backing Stores Initialized!\n");
	#endif

	restore(ps); //enable interrupts

	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	unsigned int vpno = vaddr / NBPG;
	unsigned int i = 0;
	bs_map_t *bs;
	for(; i < NBS; i++)
	{
		bs = &bsm_tab[i];
		if(bs->bs_status == BSM_UNMAPPED)
			continue; //no need to check this one

		if(bs->bs_pid[pid] == TRUE) //This pid is using this backing store
		{
			#ifdef DBG_PRINT
				kprintf("bsm_lookup i: %d\tvpno: %04X\tbs_vpno: %04X\tbs_npages: %d\n", i, vpno, bs->bs_vpno, bs->bs_npages);
			#endif

			if(vpno >= bs->bs_vpno)
			{
				if(vpno < (bs->bs_vpno + bs->bs_npages)) //The given address is inside THIS backing store
				{
					*store = i;
					*pageth = vpno - bs->bs_vpno; //the page within the backing store
					return OK; //stop looking we already found it
				}
			}
		}
	}

	#ifdef DBG_PRINT
		kprintf("bsm_lookup failed to find any backing stores that matched!\n");
	#endif

	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	#ifdef DBG_PRINT
		kprintf("Attempting to map bsm %d\n", source);
	#endif

	bs_map_t *bs = &bsm_tab[source];

	if(bs->bs_private == BSM_PRIVATE)
	{
		#ifdef DBG_PRINT
			kprintf("bsm %d is private, cannot map to it\n", source);
		#endif
		return SYSERR;
	}

	if(bs->bs_status == BSM_UNMAPPED)
	{
		//Need to map this bsm
		bs->bs_status	= BSM_MAPPED;
		bs->bs_pid[pid]	= TRUE;
		bs->bs_private	= BSM_PUBLIC;
		bs->bs_npages	= npages;
		bs->bs_vpno	= vpno;
	}
	else
	{
		//the bsm is already mapped just add this pid and update npages + vpno
		bs->bs_pid[pid]	= TRUE;
		bs->bs_npages	= npages;
		bs->bs_vpno	= vpno;
	}

	#ifdef DBG_PRINT
		kprintf("bsm %d mapped\n", source);
	#endif

	return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
}

/*-------------------------------------------------------------------------
 * print_bsm - print the bsm struct information
 *-------------------------------------------------------------------------
 */
void print_bsm(bs_map_t bs)
{
	kprintf("bs_status:\t%d\n", bs.bs_status);
	int i = 0;
	for(; i < NPROC; i++)
		kprintf("bs_pid[%d]:\t%d\n", i, bs.bs_pid[i]);
	kprintf("bs_vpno:\t%d\n", bs.bs_vpno);
	kprintf("bs_npages:\t%d\n", bs.bs_npages); 
	kprintf("bs_private:\t%d\n", bs.bs_private);
}

