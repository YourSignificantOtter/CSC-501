#include "kernel.h"
#include "stdio.h"
#include "proc.h"

#include "paging.h"

/*------------------------------------------------------------------------
 * create_page_directory - creates a page directory table for a process
 *	pid - the process ID that a directory is created for
 *	returns - the PDBR for the newly created page directory table
 *------------------------------------------------------------------------
*/
unsigned long create_page_directory(int pid)
{
	STATWORD ps;
	disable(ps); //Disable interrupts

	#ifdef DBG_PRINT
		kprintf("Creating Page Directory For Process: %d %s\n", pid, proctab[pid].pname);
	#endif

	int frame = 0;
	get_frm(&frame);
	init_frm(frame, pid, FR_DIR); // get and initialize a frame for this directory
	pd_t *pd = (pd_t *)((FRAME0 + frame) * NBPG);
	
	int i = 0;
	for(; i < NGPG; i++)
	{
		pd[i].pd_pres	= 1;
		pd[i].pd_write	= 1;
		pd[i].pd_user	= 0;
		pd[i].pd_pwt	= 0;
		pd[i].pd_pcd	= 0;
		pd[i].pd_acc	= 0;
		pd[i].pd_mbz	= 0;
		pd[i].pd_fmb	= 0;
		pd[i].pd_global	= 1;
		pd[i].pd_avail	= 0;
		pd[i].pd_base	= g_pt[i]; //Point these first 4 entries to the 4 global page tables
	}

	#ifdef DBG_PRINT
		kprintf("Created global page directory entries!\nCreating local entries\n");
	#endif

	for(i = NGPG; i < NEPG; i++) //Init the non-global entries
	{
		pd[i].pd_pres	= 0;
		pd[i].pd_write	= 1;
		pd[i].pd_user	= 0;
		pd[i].pd_pwt	= 0;
		pd[i].pd_pcd	= 0;
		pd[i].pd_acc	= 0;
		pd[i].pd_mbz	= 0;
		pd[i].pd_fmb	= 0;
		pd[i].pd_global	= 0;
		pd[i].pd_avail	= 0;
		pd[i].pd_base	= 0; 
	}

	#ifdef DBG_PRINT
		kprintf("Created local page directory entries!\n");
	#endif

	proctab[pid].pdbr = (unsigned long)pd;
	restore(ps);
	return pd; //This return is the PDBR for the newly created PD
}

/*------------------------------------------------------------------------
 * clear_page_directory - Clears a page directory, called in free_frm
 *	frameIdx - the index of the frame that holds the page dir
 *------------------------------------------------------------------------
 */
void clear_page_directory(int frameIdx)
{
	STATWORD ps;
	disable(ps);

	int i = 0;
	pd_t *pd = (pd_t *)((FRAME0 + frameIdx) * NBPG);
	for(; i < NEPG; i++)
	{
		//Fill the page with 0
		pd[i].pd_pres	= 0;
		pd[i].pd_write	= 0;
		pd[i].pd_user	= 0;
		pd[i].pd_pwt	= 0;
		pd[i].pd_pcd	= 0;
		pd[i].pd_acc	= 0;
		pd[i].pd_mbz	= 0;
		pd[i].pd_fmb	= 0;
		pd[i].pd_global	= 0;
		pd[i].pd_avail	= 0;
		pd[i].pd_base	= 0;
	}

	restore(ps);
}

/*------------------------------------------------------------------------
 * dump_page_directory - prints a processes page directory
 *	pid - the process ID of the page directory to dump
 *------------------------------------------------------------------------
 */
void dump_page_directory(int pid)
{
	STATWORD ps;
	disable(ps);

	pd_t *pd = (pd_t *)proctab[pid].pdbr;
	
	int i = 0;
	for(; i < NEPG; i++)
	{
		kprintf("Entry: %d\tContents: %08X\n", i, pd[i]);
	}

	restore(ps);	
}
