#include "kernel.h"
#include "stdio.h"
#include "proc.h"

#include "paging.h"

/*------------------------------------------------------------------------
 * pagereplace - replace a page based on the set replacement policy
 *	address - virtual address that caused the page fault
 *------------------------------------------------------------------------
 */
SYSCALL pagereplace(unsigned long address)
{
	static int sc_position = 0;
	int policy = grpolicy();

	unsigned int virtualPage = address / NBPG;

	virt_addr_t *vaddr 		= (virt_addr_t *)&address;
	unsigned int pageDirectoryIdx   = vaddr->pd_offset;
        unsigned int pageTableIdx       = vaddr->pt_offset;
        unsigned int pageOffset         = vaddr->pg_offset;

	if(policy == SC)
	{
		#ifdef DBG_PRINT
			kprintf("Performing SC Page Replacement!\n");
		#endif

		if(queueRoot->next == NULL)
		{
			kprintf("The SC queue is empty, I assume this means the frames are full of page tables and directories\n");
			return SYSERR;
		}
		c_q_entry_t *iter = queueRoot->next;
		pt_t *pt;
		int i = 0;
		do
		{
			//Start at sc_position and check if reference bit is set
			if(i == sc_position)
			{
				sc_position++;
				pt = (pt_t *)((FRAME0 + iter->data) * NBPG); //TODO this is wrong, need
				//To add something to the frm_map_t that holds the frame index to the 
				//Page table that is pointing at this page

				if(pt[pageTableIdx].pt_acc == 1)
				{
					//The page was accessed, clear that and move to the next one
					pt[pageTableIdx].pt_acc = 0;
				}
				else
				{
					//The page was NOT accessed, swap this one out!
					if(pageReplaceDebug == TRUE)
					{
						kprintf("Performing SC policy page replacement on frame: %d\n", iter->data);
					}
					
					//Mark the page table entry pointing to this page as no longer present
					pt[pageTableIdx].pt_present = 0;

					//If the page table belongs to the current process invalidate the TLB
					if(currpid == frm_tab[iter->data].fr_pid)
						write_cr3(proctab[currpid].pdbr); //Writing CR3 invalidates all of TLB

					//In inverted table decrease refcnt for page table pointing to this page
					//TODO: Use that added frm_map_t field to go into the page table frame and decrement

					//If the refcnt is now 0 mark the page directory entry that points to it as not present
					//TODO: Add a field in frm_map_t that stores the page direcotry that points to this page table
					//Could just be one field like fr_parent_frame

					//Check the dirty bit for the page
					//if(dirty == 1)
					//	find the bsm, write_bs to that bsm
					//give the frame back to the page fault handler to use					

					break; //Stop looping, the replacement is completed
				}
			}
	
			i++;
			iter = iter->next;			
			if(iter == queueRoot) //We have wrapped around to the start
			{
				#ifdef DBG_PRINT
					kprintf("SC Policy page replace has wrapped around the queue!\n");
				#endif
				i = 0;
				sc_position = 0;
				iter = queueRoot->next; //Move the iter to the start of the queue
			}
		} while(iter != queueRoot);		
	}
	else if(policy == AGING)
	{
		#ifdef DBG_PRINT
			kprintf("Performing Aging Page Replacement!\n");
		#endif
	}
	else
	{
		kprintf("Illegal replacement policy value set!\n");
		return SYSERR;
	}

	return OK;
}
