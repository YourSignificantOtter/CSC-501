/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

fr_map_t frm_tab[NFRAMES];

/*-------------------------------------------------------------------------
 * init_frm_table - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm_table()
{
	STATWORD ps;
	disable(ps); //Disable interrupts

	#ifdef DBG_PRINT
		kprintf("Initialize Frames\n");
	#endif

	int i = 0;
	for(i; i < NFRAMES; i++)
	{
		frm_tab[i].fr_status	= FRM_UNMAPPED;
		frm_tab[i].fr_pid	= NPROC;
		frm_tab[i].fr_vpno	= -1;
		frm_tab[i].fr_refcnt	= 0;
		frm_tab[i].fr_type	= FR_INIT;
		frm_tab[i].fr_dirty	= DIRTY;
		frm_tab[i].fr_age	= 0;

		#ifdef DBG_PRINT
			if(i % 100 == 0)
			{
				kprintf("frm_tab[%d]\n", i);
				print_frm(frm_tab[i]);
				kprintf("\n");
			}
		#endif
	}

	#ifdef DBG_PRINT
		kprintf("Frames are Initialized!");
	#endif

	restore(ps); //restore interrupts

	return OK;
}

/*-------------------------------------------------------------------------
 * init_frm - initialize a single frame for use
 *	frmIdx - Index into frm_tab of the frame to initialize
 *	pid - Process that will be using this frame
 *	type - type of frame this will be
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm(int frmIdx, int pid, int type)
{
	STATWORD ps;
	disable(ps);

	frm_tab[frmIdx].fr_status = FRM_MAPPED;
	frm_tab[frmIdx].fr_pid = pid;
	frm_tab[frmIdx].fr_refcnt = 1;
	frm_tab[frmIdx].fr_type = type;
	frm_tab[frmIdx].fr_dirty = CLEAN;
	frm_tab[frmIdx].fr_age = 0;

	#ifdef DBG_PRINT
		kprintf("Frame %d initialized\n", frmIdx);
		print_frm(frm_tab[frmIdx]);
	#endif

	if(type == FR_PAGE)
	{
		cq_enqueue(frmIdx, cq_queueRoot); //Enqueue the frames into a circular queue on creation for SC
		fq_enqueue(frmIdx, fq_head, fq_tail); //Enqueue the frames into a fifo queue on creation for AGING
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	*avail = -1;
	int i = 0;
	for(;i < NFRAMES; i++)
	{
		if(frm_tab[i].fr_status == FRM_UNMAPPED) // if the frame is not used
		{
			*avail = i;
			break;
		}
	}

	#ifdef DBG_PRINT
		if(*avail == -1)
			kprintf("No more frames are available!\n");
		else
			kprintf("Returned frame %d\n", *avail);
	#endif

	if(*avail == -1)
		return SYSERR;

	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("Free_frm(%d) Called by %s\n", i, proctab[currpid].pname);
	#endif

	fr_map_t fr = frm_tab[i];

	if(fr.fr_type == FR_DIR)
	{
		#ifdef DBG_PRINT
			kprintf("Freeing page directory\n");
		#endif
		clear_page_directory(i);
		clear_frm(i); //No extra work to do for page dir
	}
	else if(fr.fr_type == FR_TBL)
	{
		#ifdef DBG_PRINT
			kprintf("Freeing page table\n");
		#endif
		//Tell the page directory this page table isnt here anymore
		int pageDirectoryIdx = -1;
		pd_t *pd = (pd_t *)((FRAME0 + fr.fr_parent) * NBPG);
		if(find_page_directory_entry(i, &pageDirectoryIdx, pd) == SYSERR)
		{
			#ifdef DBG_PRINT
				kprintf("free_frm(%d) attempts to free a page table that has no related page directory entry!\n", i);
			#endif
		}
		else
		{
			pd[pageDirectoryIdx].pd_pres = 0; //Tell the page direcotry this page table is no longer present
		}
		clear_page_table(i);
		clear_frm(i); //No extra work to do for page table
	}
	else if(fr.fr_type == FR_PAGE)
	{
		#ifdef DBG_PRINT
			kprintf("Freeing page\n");
		#endif
		//Need to write page's back to the bsm where applicable
		int store = 0, pageth = 0;
		if(bsm_lookup(currpid, frm_tab[i].fr_vpno * NBPG, &store, &pageth) == SYSERR)
		{
			#ifdef DBG_PRINT
				kprintf("Call to free_frm(%d) frees a page that has no mapping!\n", i);
				kprintf("Assuming that it has been xunmapped previously!\n");
			#endif
		}
		else
		{
			//Write to the backing store
			if(frm_tab[i].fr_dirty == DIRTY)
				write_bs((char *)((FRAME0 + i) * NBPG), store, pageth);
		}
		//Tell the page table this page is no longer here
		int pageTableIdx = -1;
		pt_t *pt = (pt_t *)((FRAME0 + fr.fr_parent) * NBPG);
		if(find_page_table_entry(FRAME0 + i, &pageTableIdx, pt) == SYSERR)
		{
			#ifdef DBG_PRINT
				kprintf("free_frm(%d) attempts to free a page that has no related page table entry\n", i);
			#endif
		}
		else
		{
			pt[pageTableIdx].pt_pres = 0; //Tell the page table this page is no longer available
		}
		//Clear the data in the page
		clear_page_table(i); //Could make another clear_page() helper but this also fills it with 0 so eh
		clear_frm(i);
	}
	else
	{
		kprintf("\n\nsomething is wrong here\n");
		kprintf("Attempt to call free_frm(%d) but frame has no type!\n", i);
		return SYSERR;
	}	

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * clear_frm - helper function for free_frm 
 *-------------------------------------------------------------------------
 */
SYSCALL clear_frm(int i)
{
	fr_map_t *fr = &frm_tab[i];
	
	fr->fr_status	= FRM_UNMAPPED;
	fr->fr_pid	= NPROC;
	fr->fr_vpno	= -1;
	fr->fr_refcnt	= 0;
	fr->fr_type	= FR_INIT;
	fr->fr_dirty	= DIRTY;
	fr->fr_parent	= 0;
	fr->fr_age	= 0;
}

/*-------------------------------------------------------------------------
 * find_frm - find a specific frame
 *	pid - the process with the frame to find
 *	vpno - the virtual page number to find
 *	type - the type of frame to find
 *	frmIdx - the returned frame index 
 *-------------------------------------------------------------------------
 */
SYSCALL find_frm(int pid, int vpno, int type, int *frmIdx)
{
	*frmIdx = -1;
	int i = 0;
	for(; i <NFRAMES; i++)
	{
		//Skip if the frame isnt mapped
		if(frm_tab[i].fr_status == FRM_UNMAPPED)
			continue; 

		//Otherwise check if its mapped to the pid we are searching for
		if(frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == type && frm_tab[i].fr_vpno == vpno)
		{
			//Return this value
			*frmIdx = i;
			break;
		}
	}

	if(*frmIdx == -1)
		return SYSERR;

	return OK;
}

/*-------------------------------------------------------------------------
 * print_frm - print a frame 
 *-------------------------------------------------------------------------
 */
void print_frm(fr_map_t frame)
{
	kprintf("fr_status:\t%s\n", frame.fr_status == FRM_UNMAPPED ? "Unmapped" : "Mapped");
	kprintf("fr_pid:\t\t%d\n", frame.fr_pid);
	kprintf("fr_vpno:\t%d\n", frame.fr_vpno);
	kprintf("fr_refcnt:\t%d\n", frame.fr_refcnt);
	kprintf("fr_type:\t%s\n", frame.fr_type == FR_DIR ? "Page Directory" : frame.fr_type == FR_TBL ? "Page Table" : "Page");
	kprintf("fr_dirty:\t%s\n", frame.fr_dirty == DIRTY ? "Dirty" : "Clean");
	kprintf("fr_parent:\t%d\n", frame.fr_parent);
	kprintf("fr_age:\t\t%d\n", frame.fr_age);
}

/*-------------------------------------------------------------------------
 * print_frm_contents - print all the data in a frame char by char
 *	frameIdx - the frame whose contents will be printed 
 *-------------------------------------------------------------------------
 */
void print_frm_contents(int frameIdx)
{
	STATWORD ps;
	disable(ps);

	int i = 0;
	unsigned int *address = (unsigned char *)((FRAME0 + frameIdx) * NBPG);
	unsigned long data;
	for(; i < NEPG * 2; i++)
	{
		data = *address;
		kprintf("0x%08X: ", address);
		dump32(data);
		address++; 
	}

	restore(ps);
}
