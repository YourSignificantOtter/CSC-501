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

	unsigned long badAddr = read_cr2();
	unsigned int pageTableNum = (badAddr & PAGE_TABLE_NUM_MASK) >> PAGE_TABLE_NUM_SHIFT;
	pageTableNum *= 4; //Multiply the shifted value by 4 to get the offset into the directory
	unsigned int pageNum = (badAddr & PAGE_NUM_MASK) >> PAGE_NUM_SHIFT;
	unsigned int offset = (badAddr & OFFSET_MASK) >> OFFSET_SHIFT;

	#ifdef DBG_PRINT
		kprintf("\n\n!!!! Entered Page Fault ISR !!!!\n");
		kprintf("Process: %s\n", proctab[currpid].pname);
		kprintf("Address = %08X\tPage Table Num = %04X\tPage Num = %04X\tOffset = %04X\n", badAddr, pageTableNum, pageNum, offset);
	#endif


	pd_t *currPageDirectory = (pd_t *)proctab[currpid].pdbr; //Case the PDBR to the PD type
	//Check a is a legal address???
	pd_t legalAddr = currPageDirectory[pageTableNum];
	if(legalAddr.pd_pres == 1)
	{
		#ifdef DBG_PRINT
			kprintf("This page does exist in the current page directory!\n");
		#endif
	}
	else
	{
		#ifdef DBG_PRINT
			kprintf("This page does NOT exist int he current page directory!\n");
		#endif
	}
	


	//does the pageTableNum'th page exist?
		

	while(1)
		;

	return OK;
}


