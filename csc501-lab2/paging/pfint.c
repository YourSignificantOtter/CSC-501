/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern unsigned long pferrcode;

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD ps;
	disable(ps);

	unsigned long badAddr = read_cr2();
	
	#ifdef DBG_PRINT
		kprintf("\n\n!!!! Entered Page Fault ISR !!!!\n");
		kprintf("Process: %s\n", proctab[currpid].pname);
	#endif

	//Check if badAddr is a legal address???
	int *store, *pageth;
	if(bsm_lookup(currpid, badAddr, store, pageth) == SYSERR)
	{
		kprintf("The address %08X does not exist in any backing store or in the page directory!\n", badAddr);
		kprintf("Killing process %s!\n", proctab[currpid].pname);
		kill(currpid);
		restore(ps);
		return SYSERR; //I dont think this will ever run? but just to be safe ill leave it
	}

	/*
	unsigned int pageTableNum = (badAddr & PAGE_TABLE_NUM_MASK) >> PAGE_TABLE_NUM_SHIFT;
	pageTableNum *= 4; //Multiply the shifted value by 4 to get the offset into the directory
	unsigned int pageNum = badAddr / NBPG;
	unsigned int offset = (badAddr & OFFSET_MASK) >> OFFSET_SHIFT;
	*/

	pd_t *pdbr = (pd_t *)read_cr3();	
	virt_addr_t *vaddr = (virt_addr_t *)&badAddr;
	unsigned int p = vaddr->pd_offset;
	unsigned int q = vaddr->pt_offset;
	unsigned int pt = vaddr->pg_offset;

	//Obtain the pageTableNum'th page, if it doesnt exist obtain a frame for it an init it
	if(pdbr[p].pd_pres != 1)
	{
		#ifdef DBG_PRINT
			kprintf("Obtaining a new frame...");
		#endif
		unsigned int newFrame = 0;
		if(get_frm(&newFrame) == SYSERR)
		{
			#ifdef DBG_PRINT
				kprintf("Obtain new frame failed!\n");
			#endif
			restore(ps);
			return SYSERR;
		}
		//initialize the new frame
		init_frm(newFrame, currpid, FR_TBL);
		pt_t *newPt = (FRAME0 + newFrame) * NBPG; //Get the address for the new Page table
		init_pt(newPt);		
		
		//Update the page directory entry to point to the newly created page table
		//Anything not referenced here is already set in the create_page_directory function
		pdbr[p].pd_pres = 1;
		pdbr[p].pd_base = (unsigned int)newPt / NBPG;
	}
	else
	{ //The frame already existed in the pdbr? how did we get a page fault if this true...
		#ifdef DBG_PRINT
			kprintf("pdbr[%d].pd_pres == 1\n", p);
			kprintf("I dont know what that means\n");
		#endif
	}

	while(1)
		;

	restore(ps);
	return OK;
}


