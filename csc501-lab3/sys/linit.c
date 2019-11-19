#include <kernel.h>
#include <stdio.h>
#include <proc.h>
#include <lock.h>

/* -------------------------------------------------------
 * linit - initialize all the system locks
 * -------------------------------------------------------
 */
int linit(void)
{
	#ifdef DBG_PRINT
		kprintf("\tInitializing system R/W Locks!\n");
	#endif

	int i = 0;
	for(; i < NLOCK; i++)
	{
		locks[i].status = INIT;	//locks are initialized
		locks[i].pcount = 0;	//No processes are waiting for the lock
//		locks[i].currprio = 0;
		locks[i].currpids = 0;
		q_node_t *queue = getmem(sizeof(q_node_t));
		queue->prio = -1;
		queue->type = -1;
		queue->pid  = -1;

		locks[i].head = queue;	//Point the queue to null to start
		locks[i].tail = queue;
	}

	return OK;
}
