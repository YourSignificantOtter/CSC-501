#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

/* -------------------------------------------------------
 * lock - obtain a lock or add the process into the lock queue
 *	des1 - the lock id to obtain
 *	type - READ or WRITE, describes what the process wants the lock for
 *	priority - the priority of the process, determines its location in queue
 * -------------------------------------------------------
 */
int lock(int des1, int type, int priority)
{
	#ifdef DBG_PRINT
		kprintf("\tprocess: %s(PID: %d) attempting to take lock: %d as a %s with prio %d\n", proctab[currpid].pname, currpid, des1, type == READ ? "READER" : "WRITER", priority);
	#endif

	//Check lock ID
	if(isbadlock(des1))
	{
		#ifdef DBG_PRINT
			kprintf("\tlock: %d is not a valid lock ID!\n", des1);
		#endif
		return SYSERR;
	}

	//Check R/W Type
	if(type != READ && type != WRITE)
	{
		#ifdef DBG_PRINT
			kprintf("\tType: %d is not a valid type\n\tExpected READ or WRITE!\n");
		#endif
		return SYSERR;
	}

	STATWORD ps;
	disable(ps);

	lock_t *lk = &locks[des1];

	//Check if the lock is free
	if(lk->status == FREE)
	{
		#ifdef DBG_PRINT
			kprintf("\tThe lock is available to use immediatly\n");
		#endif

		//Use the lock!
		lk->status = type;
		lk->currprio = priority;
		lk->currpids[currpid] = TRUE;
		lk->owner = currpid;

		proctab[currpid].plocks[des1] = TRUE;

		q_enqueue(priority, type, currpid, des1);

		restore(ps);
		return OK;
	}

	//Check if the lock is currently used as read and we want to read as well
	if(lk->status == READ && type == READ)
	{
		q_enqueue(priority, type, currpid, des1);
		prio_inherit(currpid, priority, des1);

		//Check if the prio >= the highest WRITE in the queue
		q_node_t *iter = lk->head;
		while(iter != lk->tail)
		{
			if(iter->type == WRITE)
			{
				if(priority >= iter->prio)
				{
					#ifdef DBG_PRINT
						kprintf("\tNew reader added to lock\n");
					#endif
					lk->currpids[currpid] = TRUE;
					restore(ps);
					return OK;
				}
				else
				{
					#ifdef DBG_PRINT
						kprintf("\tNew reader not added to lock, still enqueued\n");
					#endif
					proctab[currpid].pstate = PRWAIT;
					proctab[currpid].pwaitret = OK;
					resched();
					break;
				}
			}
			iter = iter->next;
		}
	}

	if(type == WRITE)
	{
		#ifdef DBG_PRINT
			kprintf("\tWrite locks are exclusive so we enqueue this request regardless of priorities.\n");
		#endif

		prio_inherit(currpid, priority, des1);

//		lk->pcount++;
		q_enqueue(priority, type, currpid, des1);

		proctab[currpid].pstate = PRWAIT; //Block the current process
		proctab[currpid].pwaitret = OK; 
		resched();
	}

	restore(ps);
	return OK;
}

void print_lock(int lock)
{
	if(isbadlock(lock))
		return;

	lock_t *lk = &locks[lock];

	kprintf("\tLock: %d\tstatus: %d\tpcount: %d\n", lock, lk->status, lk->pcount);
	int i = 0;
	kprintf("\t");
	for(; i < NPROC; i++)
	{
		kprintf("currpids[%02d]: %s  ", i, lk->currpids[i] == TRUE ? "TRUE " : "FALSE");
		if((i + 1) % 5 == 0)
			kprintf("\n\t");
	}
}
