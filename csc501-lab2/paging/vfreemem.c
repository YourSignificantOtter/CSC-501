/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

#include <paging.h>

extern struct pentry proctab[];

int vmcheck_next_block_free(vh_ptr_t *currBlock);

/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(struct mblock *block, unsigned size)
{
	#ifdef DBG_PRINT
		kprintf("Attempting to free %d byte(s) of virtual heap at location 0x%08X\n", size, block);
	#endif

	//check if this process was created with vcreate
	if(proctab[currpid].store == SYSERR)
	{
		kprintf("Cannot call vfreemem from a function no virtual heap!\n");
		kprintf("Killing process %s\n", proctab[currpid].pname);
		kill(currpid);
		return SYSERR;
	}

	//Check if the given address is within the heap of the process
	if(block < proctab[currpid].vmemlist || block > proctab[currpid].vmemlist + proctab[currpid].vhpnpages * NBPG)
	{
		kprintf("Cannot free an address outside of the function heap!\n");
		kprintf("Killing proces %s\n", proctab[currpid].pname);
		kill(currpid);
		return SYSERR;
	}
	vh_ptr_t *ptr = (vh_ptr_t *)block;
	ptr = ptr - sizeof(vh_ptr_t); //shift back up to where the overhead pointer SHOULD be

	//Check if the passed poinrter is actually memory that has been allocated with vgetmem
	if(ptr->vm_magic_next != VM_MAGIC)
	{
		kprintf("The passed pointer 0x%08X is not a pointer that was allocated with vgetmem!\n", block);
		return SYSERR;
	}

	//check if the passed size if a legal value
	if(size <= 0 || size > ptr->vm_size)
	{
		kprintf("size value %d passed to vfreemem is outside of the limits 1 - %d\n", size, ptr->vm_size);
		return SYSERR;
	}

	//I dont know how to handle the following situation
	if(size != ptr->vm_size && ptr->vm_size - size < sizeof(vh_ptr_t) + 1) //+1 because need at least 1 byte for the new overhead pointer to point at
	{
		kprintf("I am not sure how to handle this case\n");
		kprintf("The block that is being freed is %d bytes, and you want to free %d of them\n", ptr->vm_size, size);
		kprintf("Doing so does not leave enough bytes for a new overhead pointer to be placed\n");
		kprintf("Remaining bytes if free occurs: %d\tsizeof(vh_ptr_t): %d\n", ptr->vm_size - size, sizeof(vh_ptr_t));
		kprintf("I will assume this is an error state for now\n");
		return SYSERR;
	}


	//Actually free the memory!
	#ifdef DBG_PRINT
		kprintf("Freeing the memory at location 0x%08x!\n", block);
		kprintf("Overhead pointer information: size %d\tmagic_next: 0x%08X\n", ptr->vm_size, ptr->vm_magic_next);
	#endif

	int concat = vmcheck_next_block_free(ptr);

	if(size == ptr->vm_size && concat == 0) //freeing the entire block but not continuous portion
	{
		#ifdef DBG_PRINT
			kprintf("Freeing the entire allocated block at 0x%08X\n", ptr);
		#endif
		//No need to touch the vm_size block, the size is the same just no longer allocated
		ptr->vm_magic_next = ptr + sizeof(vh_ptr_t) + ptr->vm_size; //Point to the next block
	}
	else if(size == ptr->vm_size && concat > 0) //freeing the entire block and a continous portion after
	{
		#ifdef DBG_PRINT
			kprintf("Free the entire allocated block at 0x%08X\n", ptr);
			kprintf("Also concatanating it with %d following free blocks!\n", concat);
		#endif

		int totalSize = 0;
		int i = 0;
		vh_ptr_t *iter = ptr;
		for(; i <= concat; i++)
		{
			totalSize += iter->vm_size;
			if(i == 0)
				iter += iter->vm_size + sizeof(vh_ptr_t);
			else
				iter = iter->vm_magic_next;
		}
		
		ptr->vm_size = totalSize;
		ptr->vm_magic_next = ptr + (sizeof(vh_ptr_t) * (concat + 1)) + totalSize;
	}
	else if(size == ptr->vm_size && concat < 0) //freeing the entire block, a continous portion after, 
	{ //and joining with the free mem head

		int check = -1 * ((proctab[currpid].vhpnpages * NBPG) / (sizeof(vh_ptr_t) + 1));
		concat = concat - check;

		#ifdef DBG_PRINT
			kprintf("Free the entire allocated block at 0x%08X\n", ptr);
			kprintf("Also concatanating it with %d following free blocks!\n", concat);
			kprintf("This will merge with the head of the free mem list\n");
		#endif

		int totalSize = 0;
		int i = 0;
		vh_ptr_t *iter = ptr;
		for(; i <= concat + 1; i++)
		{
			totalSize += iter->vm_size;
			if(i == 0)
				iter += iter->vm_size + sizeof(vh_ptr_t);
			else
				iter = iter->vm_magic_next;
		}

		ptr->vm_size = totalSize;
		ptr->vm_magic_next = 0;
	}
	else //Partially freeing a block
	{
		#ifdef DBG_PRINT
			kprintf("Freeing %d bytes of a %d allocated block\n", size, ptr->vm_size);
		#endif

		//create a new overhead pointer at the location it will be placed
		vh_ptr_t *newOH = (vh_ptr_t *)(ptr + sizeof(vh_ptr_t) + ptr->vm_size - size);
		newOH->vm_size = ptr->vm_size - size - sizeof(vh_ptr_t);
		newOH->vm_magic_next = ptr->vm_magic_next;

		ptr->vm_size -= size;
		ptr->vm_magic_next = newOH;
	}

	#ifdef DBG_PRINT
		kprintf("\n\nPrinting all the allocated memory spaces after vfree mem completes\n");
		vh_ptr_t *iter = (vh_ptr_t *)proctab[currpid].vmemlist;
		int i = 0;
		while(iter->vm_magic_next != 0)
		{
			kprintf("entry: %d\tlocation: 0x%08X\tsize: 0x%08X\tmagic_next: 0x%08X\n", i, iter, iter->vm_size, iter->vm_magic_next);
			if(iter->vm_magic_next == VM_MAGIC)
			{
				iter += iter->vm_size + sizeof(vh_ptr_t);
			}
			else
			{
				iter = (vh_ptr_t *)iter->vm_magic_next;
			}
			i++;
		}
		kprintf("entry: %d\tlocation: 0x%08X\tsize: 0x%08X\tmagic_next: 0x%08X\n", i, iter, iter->vm_size, iter->vm_magic_next);
		kprintf("\n\n");
	#endif

	return OK;
}

/*------------------------------------------------------------------------
 *  vfreemem  --  check if the following memory block(s) are free, this function is recursive
 *------------------------------------------------------------------------
 */
int vmcheck_next_block_free(vh_ptr_t *currBlock)
{
	int ret = 0;
	//Check if the NEXT block of memory is a free block of memory, if it is concatenate the two
	vh_ptr_t *test = (vh_ptr_t *)(currBlock + currBlock->vm_size + sizeof(vh_ptr_t));
	if(test->vm_magic_next != VM_MAGIC && test->vm_magic_next != 0)
	{
		ret++;
		ret += vmcheck_next_block_free(test);
	}
	else if(test->vm_magic_next == 0)
	{
		//join with the head of the free memory list
		//Signal this to the calling function by returning a negative value
		//The calling function must check the offset from the negative value to see
		//How many allocated regions exist before finding the head
		ret = -1 * ((proctab[currpid].vhpnpages * NBPG) / (sizeof(vh_ptr_t) + 1));
	}

	//return of 0 means NO continuous mem blocks to conjoin
	//return of >= 1 means there is ret number of blocks that can be joined
	//return of < 0 means there is a number of blocks to be joined AND the head of the memory list
	return ret;	
}
