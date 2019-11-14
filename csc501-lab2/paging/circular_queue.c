#include "kernel.h"
#include "paging.h"
#include "circular_queue.h"

c_q_entry_t *cq_queueRoot;

void init_circular_queue(c_q_entry_t *queueRoot)
{
	#ifdef DBG_PRINT
		kprintf("Creating a circular queue\n");
	#endif

	queueRoot->data = 0xDEADBEEF; //Magic number :)
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

int cq_dequeue(int data, c_q_entry_t *queueRoot)
{
	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("Dequeueing data %d from the circular queue\n", data);
	#endif

	c_q_entry_t *iter = queueRoot->next;
	int i = 0;

	do{
		if(iter->data == data)
		{
			//remove this entry
			c_q_entry_t *temp = iter->prev;
			temp->next = iter->next;
			temp = iter->next;
			temp->prev = iter->prev;
			//I think this is right?
			freemem((struct mblock *)iter, sizeof(c_q_entry_t *));

			return OK;
		}
		else
		{
			//Just keep swimming
			i++;
			iter = iter->next;
		}
	}while(iter != queueRoot);

	//If we got to this point the dequeue failed
	#ifdef DBG_PRINT
		kprintf("Call to cq_dequeue with data: %d failed! value not found in queue\n", data);
	#endif

	return SYSERR;

	restore(ps);
}

int cq_replace(int toReplace, int newData, c_q_entry_t *queueRoot)
{
	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("Replacing data %d with new data %d in circular queue\n", toReplace, newData);
	#endif

	c_q_entry_t *iter = queueRoot->next;
	int i = 0;

	do
	{
		if(iter->data == toReplace)
		{
			#ifdef DBG_PRINT
				kprintf("Found queue entry to replace at position %d\n", i);
			#endif

			iter->data = newData; //its that easy?
			return OK;
		}
		i++;	
		iter = iter->next;
	}while(iter != queueRoot);

	#ifdef DBG_PRINT
		kprintf("Could not find data %d in the queue, no replacement performed!\n", toReplace);
	#endif

	restore(ps);
	return SYSERR;
}

void cq_clear(c_q_entry_t *queueRoot)
{
	STATWORD ps;
	disable(ps);

	c_q_entry_t *iter = queueRoot->next;
	c_q_entry_t *next;
	int i = 0;
	do
	{
		next = iter->next;
		freemem((struct mblock *)iter, sizeof(c_q_entry_t));
		iter = next;
		i++;
	}while(iter != queueRoot);

	freemem((struct mblock *)queueRoot, sizeof(c_q_entry_t));

	restore(ps);
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
		i++;
	}while(iter != queueRoot);
	restore(ps);
}
