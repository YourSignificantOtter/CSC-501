/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>

#include <paging.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	//PA2 CHANGES
	//All frames which hold any pages should be written to backing store and be freed
	//This could be heavily optimized but thats a problem for another day
	int i = 0;
	fr_map_t *fr;
	for(; i < NFRAMES; i++)
	{
		fr = &frm_tab[i];
		if(fr->fr_pid == pid && fr->fr_type != FR_DIR)//we own this frame
		{
			if(fr->fr_type == FR_PAGE)//Might need to write this to the backing store
			{
				//Write to the appropriate backing store
				int store = 0, pageth = 0;
				if(bsm_lookup(pid, fr->fr_vpno * NBPG, &store, &pageth) == SYSERR)
				{
					#ifdef DBG_PRINT
						kprintf("Process %s does not have a backing store for frame %d!\n", proctab[pid].pname, i);
					#endif
				}
				else
				{
					//Write the frame contents back to the backing store
					write_bs((FRAME0 + i) * NBPG, store, pageth);
				}
			}
			//Free the frame
			free_frm(i);
		}
	}

	//All of its mappings should be removed from the backing store map
	i = 0;
	bs_map_t *bs;
	for(; i < NBS; i++)
	{
		bs = &bsm_tab[i];
		if(bs->bs_pid[pid] == TRUE)
		{
			xmunmap(bs->bs_vpno);
		}
	}

	//Backing stores for the processes heap should be relased
	if(proctab[pid].store != SYSERR)
	{
		free_bsm(proctab[pid].store);
	}

	//The frame used for the page directory should be released
	int directoryFrame = 0;
	find_frm(pid, proctab[pid].pdbr / NBPG, FR_DIR, &directoryFrame);
	if(directoryFrame > NGPG && directoryFrame < NFRAMES)
		free_frm(directoryFrame);
	//PA2 CHANGES

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
