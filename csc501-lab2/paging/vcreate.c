/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>

#include "paging.h"

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{

	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("Call to vcreate, function: %s\tWith %d private pages\n", name, hsize);
	#endif

	if(hsize > NPGBS || hsize <= 0)
	{
		#ifdef DBG_PRINT
			kprintf("Call to vcreate with hsize > 256 or <= 0\n");
			kprintf("Process will not be created\n");
		#endif
		restore(ps);
		return SYSERR;
	}

	int pid = create(procaddr, ssize, priority, name, nargs, args); //Actually create the process

	//find and set a backing store to act as this processes private heap
	int privateHeap; //The private backing store that will be used for this process
	if(get_private_bs(&privateHeap) == SYSERR)
	{
		#ifdef DBG_PRINT
			kprintf("Could not find a backing store to use as a private heap for this function\n");
			kprintf("Function will not be created!\n");
		#endif
		restore(ps);
		return SYSERR;
	}

	//Set the private store information in the process stucture
	struct pentry *pptr = &proctab[pid];
	pptr->store = privateHeap;

	//Map the backing store
	int pageNum = VIRTMEMSTART; //How can we handle multiple calls to vcreate?
	if(xmmap(pageNum, privateHeap, hsize) == SYSERR)
	{
		kprintf("Could not map private heap to process %s in vcreate!\n", name);
		kprintf("Killing process!\n");
		kill(pid);
		restore(ps);
		return SYSERR;
	}
	pptr->vhpno = pageNum;
	pptr->vhpnpages = hsize;
	pptr->vmemlist = (struct mblock *)(BACKING_STORE_BASE + privateHeap * BACKING_STORE_UNIT_SIZE);
	vm_ptr_t vmInfo;
	vmInfo.vm_size = pptr->vhpnpages * NBPG;
	vmInfo.vm_magic_next = 0;
	*(vm_ptr_t *)pptr->vmemlist = vmInfo;
	pptr->vhused = sizeof(vm_ptr_t);
	bsm_tab[privateHeap].bs_private = BSM_PRIVATE; //Set the backing store to be private, bsm_map defaults to public

	restore(ps);
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
