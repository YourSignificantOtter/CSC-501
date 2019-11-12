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
 * get_public_bsm - get a unmapped or public entry from the bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL get_public_bs(int* avail)
{
	int i = 0;
	*avail = -1;

	for(; i < NBS; i++)
	{
		if(bsm_tab[i].bs_status == BSM_UNMAPPED || bsm_tab[i].bs_private == BSM_PUBLIC)
		{
			*avail = i;
			break;
		}
	}

	if(*avail == -1)
		return SYSERR;

	return OK;
	
}


/*-------------------------------------------------------------------------
 * get_private_bsm - get an unmapped entry from the bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL get_private_bs(int* avail)
{
	int i = 0;
	*avail = -1;

	for(; i < NBS; i++)
	{
		if(bsm_tab[i].bs_status == BSM_UNMAPPED)
		{
			*avail = i;
			break;
		}
	}

	if(*avail == -1)
		return SYSERR;

	return OK;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	#ifdef DBG_PRINT
		kprintf("Freeing bsm_tab[%d]\n", i);
	#endif

	bsm_tab[i].bs_private	= BSM_PUBLIC;
	bsm_tab[i].bs_status	= BSM_UNMAPPED;
	int j = 0;
	for(; j < NPROC; j++)
		bsm_tab[i].bs_pid[j]	= FALSE;

	bsm_tab[i].bs_vpno	= 0;
	bsm_tab[i].bs_npages	= BSM_UNMAPPED;
	bsm_tab[i].bs_sem	= NULL;	
	
	return OK;
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
SYSCALL bsm_unmap(int pid, int vpno, int source)
{
	#ifdef DBG_PRINT
		kprintf("Attempting to unmap bsm %d\n", source);
	#endif

	bs_map_t *bs = &bsm_tab[source];

	if(bs->bs_private == BSM_PRIVATE)
	{
		#ifdef DBG_PRINT
			kprintf("Unmapping a backing store that was used as private heap\n");
		#endif
		//since it was a private backing store no other process could have used it
		//Can safely set to unmapped without worrying about another process losing data
		free_bsm(source);
		return OK;
	}

	int i = 0;
	int check = 0;
	for(; i < NPROC; i++) //Check if other processes are using the backing store
	{
		//in Kernel.h FALSE is defined as 0 and TRUE is defined as 1
		check += (int) bs->bs_pid[i];
	}

	if(check == 1) //Currpid is the only pid using this backing store
	{
		#ifdef DBG_PRINT
			kprintf("Unmapping a backing store that was used by a single process\n");
		#endif
		//Since no other process was using this we can safely unmap
		free_bsm(source);
		return OK;
	}
	
	//More than one process is mapped to this backing store
	//Simply remove the passed pid from the backing store map
	bs->bs_pid[pid] = FALSE;
	return OK;
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
	{
		kprintf("%02d  ", i);
	}

	kprintf("\n");
	i = 0;
	for(; i < NPROC; i++)
	{
		kprintf("%02d  ", bs.bs_pid[i]);			
	}

	kprintf("\n");
	kprintf("bs_vpno:\t%d\n", bs.bs_vpno);
	kprintf("bs_npages:\t%d\n", bs.bs_npages); 
	kprintf("bs_private:\t%d\n", bs.bs_private);
}

