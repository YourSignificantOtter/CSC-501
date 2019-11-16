#include "kernel.h"
#include "stdio.h"
#include "proc.h"

#include "paging.h"

/*------------------------------------------------------------------------
 * pagereplace - replace a page based on the set replacement policy
 *	address - virtual address that caused the page fault
 *------------------------------------------------------------------------
 */
SYSCALL pagereplace(unsigned long address, unsigned int *freePageIdx)
{
	STATWORD ps;
	disable(ps);

	static int sc_position = 0;
	int policy = grpolicy();
	unsigned int done = 0;

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

		if(cq_queueRoot->next == NULL)
		{
			cq_print(cq_queueRoot);
			kprintf("The SC queue is empty, I assume this means the frames are full of page tables and directories\n");
			restore(ps);
			return SYSERR;
		}
		c_q_entry_t *iter = cq_queueRoot->next;
		pt_t *pt;
		int pt_frame = 0;
		int i = 0;
		do
		{
			//Start at sc_position and check if reference bit is set
			if(i == sc_position)
			{
				sc_position++;
				pt_frame = frm_tab[iter->data].fr_parent; //The parent frame of this page is the
				//frame that holds the page table entry pointing to it
				pt = (pt_t *)((FRAME0 + pt_frame) * NBPG); 

				#ifdef DBG_PRINT
					kprintf("page frame: %d\tpage table frame: %d\n", iter->data, pt_frame);
				#endif;

				if(pt[pageTableIdx].pt_acc == 1)
				{
					#ifdef DBG_PRINT
						kprintf("Page was accessed, clear access bit and move on\n");
					#endif
					pt[pageTableIdx].pt_acc = 0;
				}
				else
				{
					#ifdef DBG_PRINT
						kprintf("The page was NOT accessed, swap this one out!\n");
					#endif

					if(pageReplaceDebug == TRUE)
					{
						kprintf("Performing SC policy page replacement on frame: %d\n", iter->data);
					}
					
					//Mark the page table entry pointing to this page as no longer present
					pt[pageTableIdx].pt_pres = 0;

					//In inverted table decrease refcnt for page table pointing to this page
					frm_tab[pt_frame].fr_refcnt--;

					//If the refcnt is now 0 mark the page directory entry that points to it as not present
					if(frm_tab[pt_frame].fr_refcnt <= 0)
					{
						#ifdef DBG_PRINT
							kprintf("Page table in frame %d now has no valid entries\n", pt_frame);
							kprintf("Invalidating the page direcotry entry pointing to it\n");
						#endif
						
						pd_t *pdbr = (pd_t *)read_cr3;	
						pdbr[pageDirectoryIdx].pd_pres = 0;
					}

					//Check the dirty bit for the page
					cq_replace(iter->data, iter->data, cq_queueRoot);
					free_frm(iter->data);
					*freePageIdx = iter->data;		
					done = 1;
					break; //Stop looping, the replacement is completed
				}
			}
	
			i++;
			iter = iter->next;			
			if(iter == cq_queueRoot) //We have wrapped around to the start
			{
				#ifdef DBG_PRINT
					kprintf("SC Policy page replace has wrapped around the queue!\n");
				#endif
				i = 0;
				sc_position = 0;
				iter = cq_queueRoot->next; //Move the iter to the start of the queue
			}
		} while(!done);		
	}
	else if(policy == AGING)
	{
		#ifdef DBG_PRINT
			kprintf("Performing Aging Page Replacement!\n");
		#endif
		
		if(fq_head->next == fq_tail)
		{
			#ifdef DBG_PRINT
				kprintf("FIFO Queue for aging replacement is empty!\n");
			#endif
			restore(ps);
			return SYSERR;
		}

		f_q_entry_t *iter = fq_head->next;
		int i = 0;
		pt_t *pt;
		int pt_frame = 0;
		int minAge = MAXINT;
		int minAgeIdx = 0;
		int minAgeFrame = 0;
		do
		{
			frm_tab[iter->data].fr_age /= 2;
			//Check if pt_acc has been set
			pt_frame = frm_tab[iter->data].fr_parent; //The parent frame of this page is the
			//frame that holds the page table entry pointing to it
			pt = (pt_t *)((FRAME0 + pt_frame) * NBPG); 

			#ifdef DBG_PRINT
				kprintf("page frame: %d\tpage table frame: %d\n", iter->data, pt_frame);
			#endif;

			if(pt[pageTableIdx].pt_acc == 1)
			{
				#ifdef DBG_PRINT
					kprintf("Page was accessed, add 128 to its age\n");
				#endif
				frm_tab[iter->data].fr_age += 128;
				pt[pageTableIdx].pt_acc = 0;
				if(frm_tab[iter->data].fr_age > 255)
					frm_tab[iter->data].fr_age = 255;
			}

			//Now check who has the lowest age
			if(frm_tab[iter->data].fr_age < minAge)
			{
				minAge = frm_tab[iter->data].fr_age;
				minAgeIdx = i;
				minAgeFrame = iter->data;
			}
			
			iter = iter->next;
			i++;
		}while(iter != fq_tail);
		
		#ifdef DBG_PRINT
			kprintf("Minimum age is %d at index %d in frame %d\n", minAge, minAgeIdx, minAgeFrame);
		#endif

		if(pageReplaceDebug == TRUE)
		{
			kprintf("Performing AGING policy page replacement on frame: %d\n", minAgeFrame);
		}

		//Mark the page table entry pointing to this page as no longer present
		pt[pageTableIdx].pt_pres = 0;

		//In inverted table decrease refcnt for page table pointing to this page
		frm_tab[pt_frame].fr_refcnt--;

		//If the refcnt is now 0 mark the page directory entry that points to it as not present
		if(frm_tab[pt_frame].fr_refcnt <= 0)
		{
			#ifdef DBG_PRINT
				kprintf("Page table in frame %d now has no valid entries\n", pt_frame);
				kprintf("Invalidating the page direcotry entry pointing to it\n");
			#endif
			
			pd_t *pdbr = (pd_t *)read_cr3;	
			pdbr[pageDirectoryIdx].pd_pres = 0;
		}
		fq_dequeue(minAgeFrame, fq_head, fq_tail);
		fq_enqueue(minAgeFrame, fq_head, fq_tail); //Put it back in at the end of the fifo queue
		free_frm(minAgeFrame);
		*freePageIdx = minAgeFrame;		
	}
	else
	{
		kprintf("Illegal replacement policy value set!\n");
		restore(ps);
		return SYSERR;
	}	

	#ifdef DBG_PRINT
		kprintf("Page replacement with policy %s complete!\n", policy == SC ? "SC" : "Aging");
	#endif

	write_cr3(proctab[currpid].pdbr); //Writing CR3 invalidates all of TLB
	//Big performance hit but just need something functioning for now
	
	restore(ps);
	return OK;
}
