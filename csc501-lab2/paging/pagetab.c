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













