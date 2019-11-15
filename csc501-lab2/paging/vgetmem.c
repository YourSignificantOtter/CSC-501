/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *	nbytes - the number of bytes to be allocated
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(unsigned int nbytes)
{
	#ifdef DBG_PRINT
		kprintf("Attempting to allocate %d bytes of process %s's virtual heap\n", nbytes, proctab[currpid].pname);
	#endif

	//Check if this proces was created with vcreate
	if(proctab[currpid].store == SYSERR)
	{
		kprintf("Cannot call vgetmem from a function with no virtual heap!\n");
		kprintf("Killing process %s\n", proctab[currpid].pname);
		kill(currpid);
		return( (WORD *)SYSERR);
	}

	unsigned int totalHeapSize = proctab[currpid].vhpnpages * NBPG - sizeof(vm_ptr_t);

	//Check if the passed nbytes is a legal value
	if(nbytes <= 0 || nbytes >= totalHeapSize - sizeof(vm_ptr_t))
	{
		kprintf("nbytes value %d passed to vgetmem is outside of the bounds 1 - %d\n", nbytes, totalHeapSize);
		return ( (WORD *)SYSERR);
	}

	//Check if this call is even possible regardless of the fragmentation of the free list
	if(nbytes > totalHeapSize - proctab[currpid].vhused - sizeof(vm_ptr_t))
	{
		kprintf("not enough free bytes to fulfill call to vgetmem of size %d\n", nbytes);
		return( (WORD *)SYSERR);
	}

	STATWORD ps;
	disable(ps);	

	#ifdef DBG_PRINT
		kprintf("Begin searching the free list for an available memory block\n");
		kprintf("Attempted to implement a Best-fit allocation\n");
	#endif

	vm_ptr_t *iter = (vm_ptr_t *)proctab[currpid].vmemlist;
	vm_ptr_t *new = iter;
	int i = 0;
	int done = 0;
	unsigned int min = MAXINT;

	while(!done)
	{
		#ifdef DBG_PRINT
			kprintf("\titer: 0x%08X\tsize: %d\tmagic_next: 0x%08X\n", iter, iter->vm_size, iter->vm_magic_next);
		#endif

		//This block is already allocated, check the next block
		if(iter->vm_magic_next == VM_MAGIC)
		{
			iter += iter->vm_size + sizeof(vm_ptr_t); //Move to the next position
		}
		else //This block is not allocated, use best-fit allocation check to see if its the min size that works
		{
			if(iter->vm_size <= min && iter->vm_size >= nbytes + sizeof(vm_ptr_t))
			{
				#ifdef DBG_PRINT
					kprintf("New minimum is at list location 0x%08X index %d\n", iter, i);
				#endif
				new = iter;
				min = iter->vm_size;	
			}
			//Move to the next position (if not already at the end)
			if(iter->vm_magic_next == 0)
			{
				done = 1;
				break;
			}
			else
				iter = (vm_ptr_t *)iter->vm_magic_next;
		}
		i++;
	}

	if(min == MAXINT)
	{
		//too much fragmentation cannot fit this piece even though there is technically enough bytes throughout
		kprintf("There is no free space large enough to allocate %d bytes\n", nbytes);
		kprintf("Likely there is too much internal fragmentation\n");
		return( (WORD *)SYSERR);
	}	

	vm_ptr_t *head = (vm_ptr_t *)(new + nbytes + sizeof(vm_ptr_t));
	head->vm_magic_next = new->vm_magic_next; //Get the new pointer to NEXT
	head->vm_size = new->vm_size - nbytes - sizeof(vm_ptr_t); //Get the new size
	
	//Set the size and magic number for the newly allocated block
	new->vm_size = nbytes;
	new->vm_magic_next = VM_MAGIC;

	#ifdef DBG_PRINT
		kprintf("allocation succesfull!\n");
	#endif

	proctab[currpid].vhused += nbytes + sizeof(vm_ptr_t);

	restore(ps);
	return( (WORD *)(new + sizeof(vm_ptr_t)));
}


