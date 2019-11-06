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

	#ifdef DBG_PRINT
		kprintf("Frame %d initialized\n", frmIdx);
		print_frm(frm_tab[frmIdx]);
	#endif

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

  kprintf("To be implemented!\n");
  return OK;
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
	kprintf("fr_status:\t%d\n", frame.fr_status);
	kprintf("fr_pid:\t\t%d\n", frame.fr_pid);
	kprintf("fr_vpno:\t%d\n", frame.fr_vpno);
	kprintf("fr_refcnt:\t%d\n", frame.fr_refcnt);
	kprintf("fr_type:\t%d\n", frame.fr_type);
	kprintf("fr_dirty:\t%d\n", frame.fr_dirty);
}

void print_frm_contents(int frameIdx)
{
	STATWORD ps;
	disable(ps);

	int i = 0;
	unsigned char *address = (unsigned char *)((FRAME0 + frameIdx) * NBPG);
	unsigned char data;
	for(; i < NEPG * 2; i++)
	{
		data = *address;
		kprintf("0x%08X: %c\n", address, data);
		address++; 
	}

	restore(ps);
}
