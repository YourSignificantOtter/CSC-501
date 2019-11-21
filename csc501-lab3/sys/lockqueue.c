#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

/* -------------------------------------------------------
 * q_enqueue - place a new entry into the queue based on priority
 *	prio - the priority of the process placed into the queue
 *	type - the READ/WRITE type of the process
 *	pid  - the PID of the process to emplace
 *	lock - the lock id that contains the queue to emplace in
 * returns the position of the queue the enqueued element is in
 * or SYSERR in case of an error
 * -------------------------------------------------------
 */
int q_enqueue(int prio, int type, int pid, int lock)
{
	if(isbadlock(lock))
	{
		#ifdef DBG_PRINT
			kprintf("\tCannot enqueue into lock %d, lock ID is not valid!\n", lock);
		#endif
		return SYSERR;
	}

	if(type != READ && type != WRITE)
	{
		#ifdef DBG_PRINT
			kprintf("\tCannot enqueue into lock %d with type %d, type is not valid!\n\tExpect READ or WRITE!\n", lock, type);
		#endif
		return SYSERR;
	}

	if(isbadpid(pid))
	{
		#ifdef DBG_PRINT
			kprintf("\tCannot enqueue into lock %d with pid %d, pid is not valid!\n", lock, pid);
		#endif
		return SYSERR;
	}

	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("\tEnqueueing prio: %d type: %s pid %d into lock %d\n", prio, type == READ ? "READ" : "WRITE", pid, lock);
	#endif

	//Get memory for the entry and fill in its data
	q_node_t *entry  = getmem(sizeof(q_node_t));
	entry->prio = prio;
	entry->pid = pid;
	entry->type = type;

	//Traverse the queue and find where this input goes (based on priority)
	q_node_t *iter = locks[lock].head;

	if(iter->pid == -1) //Empty Queue
	{
		#ifdef DBG_PRINT
			kprintf("\tLock %d's queue was empty, placing entry at the head & tail of queue \n", lock);
		#endif

		freemem((struct mblock *)iter, sizeof(q_node_t));

		locks[lock].head = entry;
		locks[lock].tail = entry;

		entry->next = NULL;
		entry->prev = NULL;

		//TODO: Update the current information for the lock here?
		restore(ps);
		return 0;
	}

	//Non-empty queue
	int i = 0;
	int done = 0;
	do
	{
		if(entry->prio > iter->prio)
		{
			if(iter == locks[lock].head) //replacing the head
			{
				locks[lock].head = entry;
				entry->prev = NULL;
				entry->next = iter;
				iter->prev = entry;				
			}
			else
			{
				q_node_t *temp = iter->prev;
				temp->next = entry;
				entry->prev = temp;
				entry->next = iter;
				iter->prev = entry;
			}
			done = 1;
		}

		if(iter == locks[lock].tail && !done) //Replace the tail
		{
			iter->next = entry;
			entry->prev = iter;
			entry->next = NULL;
			locks[lock].tail = entry;
			done = 1;
		}
		if(!done)
		{
			i++;
			iter = iter->next;
		}
	}while(!done);//iter->next != NULL); // != locks[lock].tail);

	#ifdef DBG_PRINT
		kprintf("\tEnqueue successful, new entry placed at index %d\n\tQueue after enqueue\n", i);
		q_print(lock);
	#endif

	restore(ps);
	return OK;

/*
	//Insert @ the tail
	iter->next = entry;
	entry->prev = iter;
	entry->next = NULL;
	locks[lock].tail  = entry;

	#ifdef DBG_PRINT
		kprintf("\tEnqueue successful, new entry placed at index %d\n\tQueue after enqueue\n", i+1);
		q_print(lock);
	#endif
*/
}

/* -------------------------------------------------------
 * q_dequeue - remove an entry from the queue
 *      prio - the priority of the process
 *      type - the READ/WRITE type of the process
 *      pid  - the PID of the process to remove
 *      lock - the lock id that contains the queue to remove from
 * returns the position of the queue the removed element was in
 * or SYSERR in case of an error
 * -------------------------------------------------------
 */
int q_dequeue(int prio, int type, int pid, int lock)
{
	if(isbadlock(lock))
	{
		#ifdef DBG_PRINT
			kprintf("\tCannot dqqueue from lock %d, lock ID is not valid!\n", lock);
		#endif
		return SYSERR;
	}

	if(type != READ && type != WRITE)
	{
		#ifdef DBG_PRINT
			kprintf("\tCannot dequeue from lock %d with type %d, type is not valid!\n\tExpect READ or WRITE!\n", lock, type);
		#endif
		return SYSERR;
	}

	if(isbadpid(pid))
	{
		#ifdef DBG_PRINT
			kprintf("\tCannot denqueue from lock %d with pid %d, pid is not valid!\n", lock, pid);
		#endif
		return SYSERR;
	}

	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("\tDenqueueing prio: %d type: %s pid %d into lock %d\n", prio, type == READ ? "READ" : "WRITE", pid, lock);
	#endif
	
	int i = 0;
	q_node_t *iter = locks[lock].head;
	while(iter != locks[lock].tail)
	{
		if(iter->prio == prio && iter->type == type && iter->pid == pid) //Remove this one
		{
			q_node_t *prev = iter->prev;
			q_node_t *next = iter->next;

			prev->next = next;
			next->prev = prev;

			iter->next = NULL;
			iter->prev = NULL;

			freemem((struct mblock *)iter, sizeof(q_node_t));
			#ifdef DBG_PRINT
				kprintf("\tDequeue successful, entry removed from index %d\n\tQueue after dequeue\n", i);
				q_print(lock);
			#endif
			restore(ps);
			return i;
		}
		i++;
		iter = iter->next;
	}

	#ifdef DBG_PRINT
		kprintf("\tDequeue somehow reached the end of the function?\n");
	#endif

	restore(ps);
	return SYSERR;
}

void q_print(int lock)
{
	STATWORD ps;
	disable(ps);

	if(isbadlock(lock))
		return;

	int i = 0;
	q_node_t *iter = locks[lock].head;
	while(iter != locks[lock].tail)
	{
		kprintf("\tLock: %d queue index: %d prio: %d type: %s pid: %d\n", lock, i, iter->prio, iter->type == READ ? "READ " : "WRITE", iter->pid);
		i++;
		iter = iter->next;
	}

	iter = locks[lock].tail;
	kprintf("\tLock: %d queue index: %d prio: %d type: %s pid: %d\n", lock, i, iter->prio, iter->type == READ ? "READ " : "WRITE", iter->pid);

	restore(ps);
}
