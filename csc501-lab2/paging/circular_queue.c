#include "kernel.h"
#include "paging.h"
#include "circular_queue.h"

c_q_entry_t *queueRoot;

void init_circular_queue(c_q_entry_t *queueRoot)
{
	#ifdef DBG_PRINT
		kprintf("Creating a circular queue\n");
	#endif

	queueRoot->data = SYSERR;
	queueRoot->next = NULL;
	queueRoot->prev = NULL;
}

int cq_enqueue(int data, c_q_entry_t *queueRoot)
{
	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("Enqueing value %d into the circular queue\n", data);
	#endif

	c_q_entry_t *entry = getmem(sizeof(c_q_entry_t));
	if(queueRoot->prev != NULL)
	{
		c_q_entry_t *temp = queueRoot->prev;
		temp->next = entry;
		entry->prev = temp;
	}
	else //Empty Queue
	{
		queueRoot->next = entry;
		entry->prev = queueRoot;
	}
	entry->data = data;
	entry->next = queueRoot;
	queueRoot->prev = entry;

	#ifdef DBG_PRINT
		kprintf("Queue after enqueue completes!\n");
		cq_print(queueRoot);
	#endif

	restore(ps);
}

int cq_dequeue(int index, c_q_entry_t *queueRoot)
{
	#ifdef DBG_PRINT
		kprintf("Dequeueing index %d from the circular queue\n", index);
	#endif
}

void cq_print(c_q_entry_t *queueRoot)
{
	STATWORD ps;
	disable(ps);
	c_q_entry_t *iter = queueRoot;
	int i = 0;
	do{
		kprintf("%d'th entry data: %d\n", i, iter->data);
		iter = iter->next;
	}while(iter != queueRoot);
	restore(ps);
}
