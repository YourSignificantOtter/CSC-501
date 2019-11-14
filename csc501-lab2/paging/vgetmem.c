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

	

	kprintf("To be implemented!\n");
	return( SYSERR );
}


