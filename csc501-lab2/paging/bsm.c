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
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
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
	kprintf("bs_pid:\t\t%d\n", bs.bs_pid);
	kprintf("bs_vpno:\t%d\n", bs.bs_vpno);
	kprintf("bs_npages:\t%d\n", bs.bs_npages); 
	kprintf("bs_private:\t%d\n", bs.bs_private);
}

