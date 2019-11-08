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
	unsigned int virtualPage = badAddr / NBPG;
	
	#ifdef DBG_PRINT
		kprintf("\n\n!!!! Entered Page Fault ISR !!!!\n");
		kprintf("Process: %s", proctab[currpid].pname);
		kprintf("\tvirtual address :0x%08X\n", badAddr);
	#endif

	//Check if badAddr is a legal address???
	int store, pageth;
	if(bsm_lookup(currpid, badAddr, &store, &pageth) == SYSERR)
	{
		kprintf("The address %08X does not exist in any backing store or in the page directory!\n", badAddr);
		kprintf("Killing process %s!\n", proctab[currpid].pname);
		kill(currpid);
		restore(ps);
		return SYSERR;
	}

	//Get the pdbr and the portions of the bad virtual address
	pd_t *pdbr = (pd_t *)read_cr3();	
	virt_addr_t *vaddr		= (virt_addr_t *)&badAddr;
	unsigned int pageDirectoryIdx	= vaddr->pd_offset;
	unsigned int pageTableIdx	= vaddr->pt_offset;
	unsigned int pageOffset		= vaddr->pg_offset;

//	kprintf("pageDirectoryIdx: %d\tpageTableIdx: %d\tpageOffset: %d\n", pageDirectoryIdx, pageTableIdx, pageOffset);
//	dump_page_directory(currpid);


	pt_t *newPt;

	//Check if the page directory that the bad virtual addr points to exists
	if(pdbr[pageDirectoryIdx].pd_pres != 1)
	{
		#ifdef DBG_PRINT
			kprintf("The page directory does not exist\nObtaining a new frame for the page table\n");
			kprintf("And updating the page directory to point to that table\n");
		#endif;

		//Obtain the new frame
		unsigned int newFrame = 0;
		if(get_frm(&newFrame) == SYSERR)
		{
			#ifdef DBG_PRINT
				kprintf("Obtain new frame failed!\n");
				kprintf("Implement page replacement policies!\n");
			#endif
			pagereplace(badAddr);
		}

		//initialize the new frame as a page table
		init_frm(newFrame, currpid, FR_TBL);
		newPt = (FRAME0 + newFrame) * NBPG; //Get the address for the new Page table
		init_pt(newPt);//Initialize the page table
		
		//Update the page directory entry to point to the newly created page table
		//Anything not referenced here is already set in the create_page_directory function
		pdbr[pageDirectoryIdx].pd_pres = 1;
		pdbr[pageDirectoryIdx].pd_base = (unsigned long)newPt / NBPG;
	}
	else
	{
		//Need to refcount++ the page table frame
		int frame = pdbr[pageDirectoryIdx].pd_base - FRAME0;
		frm_tab[frame].fr_refcnt++;
	}

	#ifdef DBG_PRINT
		kprintf("The page directory entry exists\n");
		kprintf("Check if the page table exists\n");
	#endif

	//Check if the page that we are trying to access is present
	pt_t *pageTable = (pt_t *)(pdbr[pageDirectoryIdx].pd_base * NBPG);
	int pageFrame = -1;

//	if(pageTable[pageTableIdx].pt_pres != 1)
	if(find_frm(currpid, virtualPage, FR_PAGE, &pageFrame) == SYSERR)
	{ 
		//The page table entry is NOT present
		#ifdef DBG_PRINT
			kprintf("The page directory exists, but the page table entry it points to is not valid\n");
			kprintf("Obtaining a new frame to utilize as the page for this data\n");
		#endif

		unsigned int pageFrame = 0;
		if(get_frm(&pageFrame) == SYSERR)
		{
			#ifdef DBG_PRINT
				kprintf("Ran out of frames\n");
				kprintf("Perform page replacement\n");
			#endif
			pagereplace(badAddr);
		}

		//Initialize the frame to be a page
		init_frm(pageFrame, currpid, FR_PAGE);
		frm_tab[pageFrame].fr_vpno = virtualPage;

		//Copy the backing store information into the new page
		read_bs((FRAME0 + pageFrame) * NBPG, store, pageth);

	 	//Update the page table entry to show the new page table 
		pageTable[pageTableIdx].pt_pres = 1;
		pageTable[pageTableIdx].pt_write = 1;
		pageTable[pageTableIdx].pt_base = (FRAME0 + pageFrame);
	}
	else
	{
		frm_tab[pageFrame].fr_refcnt++;
	}

	#ifdef DBG_PRINT
		kprintf("The page table entry exists\n");
	#endif
	
	if(find_frm(currpid, virtualPage, FR_PAGE, &pageFrame) == SYSERR)
	{
		#ifdef DBG_PRINT
			kprintf("The page frame could not be found!\n");
		#endif
		while(1)
			;
	}

	#ifdef DBG_PRINT
		kprintf("Address: 0x%08X\n", badAddr);
		kprintf("pdbr: 0x%08X\tpdbr[pageDirectoryIdx].pd_base: 0x%08X\n", pdbr, pdbr[pageDirectoryIdx].pd_base);
		kprintf("Pt: 0x%08X\tpt[pageTableIdx].pt_base: 0x%08X\n", pageTable, pageTable[pageTableIdx].pt_base);
		kprintf("page: 0x%08X\n", (FRAME0 + pageFrame));
	#endif

	write_cr3(pdbr);
	restore(ps);
	return OK;
}


