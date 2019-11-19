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
		kprintf("\tprocess: %s attempting to take lock: %d as a %s with prio %d\n", proctab[currpid].pname, des1, type == READ ? "READER" : "WRITER", priority);
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
//		lk->currprio = priority;
		lk->currpids = (1 << currpid);

		print_lock(des1);

		restore(ps);
		return OK;
	}

	//Check if the lock is currently used as read and we want to read as well
	if(lk->status == READ && type == READ)
	{
		#ifdef DBG_PRINT
			kprintf("\tLock is currently used for reading and we want to read too, feel free to read!\n");
		#endif

		prio_inherit(currpid, des1);

		q_enqueue(priority, type, currpid, des1);
		lk->currpids |= (1 << currpid);
		//Allow multiple readers by not blocking this process and not calling resched

		print_lock(des1);

		restore(ps);
		return OK;
	}

	if(type == WRITE)
	{
		#ifdef DBG_PRINT
			kprintf("\tWrite locks are exclusive so we enqueue this request regardless of priorities.\n");
		#endif

		print_lock(des1);

		prio_inherit(currpid, des1);

		lk->pcount++;
		q_enqueue(priority, type, currpid, des1);
		proctab[currpid].pstate = PRWAIT; //Block the current process
		resched();
	}

	restore(ps);
	return SYSERR;
}

void print_lock(int lock)
{
	if(isbadlock(lock))
		return;

	lock_t *lk = &locks[lock];

//	kprintf("Lock: %d\tstatus: %d\tpcount: %d\tcurrprio: %d\tcurrpids: 0x%08X\n", lock, lk->status, lk->pcount, lk->currprio, lk->currpids);
	kprintf("Lock: %d\tstatus: %d\tpcount: %d\tcurrpids: 0x%08X\n", lock, lk->status, lk->pcount, lk->currpids);
}
