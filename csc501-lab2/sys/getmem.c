/* getmem.c - getmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * getmem  --  allocate heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD *getmem(unsigned nbytes)
{
	STATWORD ps;    
	struct	mblock	*p, *q, *leftover;

	disable(ps);
	if (nbytes==0 || memlist.mnext== (struct mblock *) NULL) {
		kprintf("get mem first return\n");
		restore(ps);
		return( (WORD *)SYSERR);
	}
	nbytes = (unsigned int) roundmb(nbytes);
	for (q= &memlist,p=memlist.mnext ;
	     p != (struct mblock *) NULL ;
	     q=p,p=p->mnext)
		if ( p->mlen == nbytes) {
			q->mnext = p->mnext;
			restore(ps);
			return( (WORD *)p );
		} else if ( p->mlen > nbytes ) {
			leftover = (struct mblock *)( (unsigned)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			restore(ps);
			return( (WORD *)p );
		}
	kprintf("Second error return\n");
	restore(ps);
	return( (WORD *)SYSERR );
}
