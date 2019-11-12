#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*---------------------------------------------------------------------------
 * init_pt - initializes a page table
 * 	*pt - the page table to initialize
 *---------------------------------------------------------------------------
 */
SYSCALL init_pt(pt_t *pt)
{
	STATWORD ps;
	disable(ps);

	unsigned int i = 0;
	for(; i < NEPG; i++)
	{
		pt[i].pt_pres	= 0;
		pt[i].pt_write	= 0;
		pt[i].pt_user	= 0;
		pt[i].pt_pwt	= 0;
		pt[i].pt_pcd	= 0;
		pt[i].pt_acc	= 0;
		pt[i].pt_dirty	= CLEAN;
		pt[i].pt_mbz	= 0;
		pt[i].pt_global = 0;
		pt[i].pt_avail	= 0;
		pt[i].pt_base	= 0;
	}

	restore(ps);
	return OK;
}

/*---------------------------------------------------------------------------
 * find_page_table_entry - Search a page table for a specific vpno entry
 * 	vpno - the virtual page number to search for
 *	*pageTableIdx - the page table index of the found entry
 *	*pt - the base of the page table to search through
 *---------------------------------------------------------------------------
 */
SYSCALL find_page_table_entry(int vpno, int *pageTableIdx, pt_t *pt)
{
	STATWORD ps;
	disable(ps);

	int i = 0;
	for(; i < NEPG; i++)
	{
		if(pt[i].pt_base == vpno)
		{
			*pageTableIdx = i;
			return OK;
		}
	}

	#ifdef DBG_PRINT
		kprintf("find_page_table_entry(%d, %d, 0x%08X) did not find an entry!\n", vpno, *pageTableIdx, pt);
	#endif

	*pageTableIdx = SYSERR;
	restore(ps);
	return SYSERR;
}

/*---------------------------------------------------------------------------
 * clear_page_table - clears the data in a page table, called in free_frm
 * 	frameIdx - the index of the frame that holds the page table
 *---------------------------------------------------------------------------
 */
SYSCALL clear_page_table(int frameIdx)
{
	pt_t *pt = (pt_t *)((FRAME0 + frameIdx) * NBPG);
	init_pt(pt);
	return OK;
}
