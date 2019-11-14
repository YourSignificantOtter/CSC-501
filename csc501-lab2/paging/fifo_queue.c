#include "kernel.h"
#include "paging.h"
#include "fifo_queue.h"

f_q_entry_t *fq_head;
f_q_entry_t *fq_tail;

void init_fifo_queue(f_q_entry_t *queueHead, f_q_entry_t *queueTail)
{
	#ifdef DBG_PRINT
		kprintf("Creating a FIFO Queue\n");
	#endif

	queueHead->data = MAXINT;
	queueHead->next = queueTail;
	queueHead->prev = NULL;

	queueTail->data = MININT;
	queueTail->next = NULL;
	queueTail->prev = queueHead;
}

int fq_enqueue(int data, f_q_entry_t *queueHead, f_q_entry_t *queueTail)
{
	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("Enqueueing value %d into the fifo queue\n", data);
	#endif

	f_q_entry_t *entry = getmem(sizeof(f_q_entry_t));
	entry->data = data;

	if(queueHead->next == queueTail)
	{
		queueHead->next = entry;
		entry->prev = queueHead;
		entry->next = queueTail;
		queueTail->prev = entry;
	}
	else
	{
		f_q_entry_t *temp = queueTail->prev;
		temp->next = entry;
		entry->prev = temp;
		entry->next = queueTail;
		queueTail->prev = entry;
	}

	#ifdef DBG_PRINT
		kprintf("Queue after enqueue completes\n");
		fq_print(queueHead, queueTail);
	#endif

	restore(ps);
	return OK;
}

int fq_dequeue(int data, f_q_entry_t *queueHead, f_q_entry_t *queueTail)
{
	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("Dequeueing data %d from the fifo queue\n", data);
	#endif

	f_q_entry_t *iter = queueHead->next;
	int i = 0;

	do
	{
		if(iter->data == data)
		{
			//remove this entry
			f_q_entry_t *temp = iter->prev;
			temp->next = iter->next;
			temp = iter->next;
			temp->prev = iter->prev;
			//I think this is right?
			freemem((struct mblock *)iter, sizeof(f_q_entry_t *));

			return OK;
		}
		else
		{
			//Just keep swimming
			i++;
			iter = iter->next;
		}
	}while(iter != queueTail);

	//If we got to this point the dequeue failed
	#ifdef DBG_PRINT
		kprintf("CAll to fq_dequeue with data %d failed! value not found in queue\n", data);
	#endif

	restore(ps);
	return SYSERR;
}

int fq_replace(int toReplace, int newData, f_q_entry_t *queueHead, f_q_entry_t *queueTail)
{
	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
			kprintf("Replacing data %d with new data %d in fifo queue\n", toReplace, newData);
	#endif

	f_q_entry_t *iter = queueHead->next;
	int i = 0;

	do
	{
		if(iter->data == toReplace)
		{
			#ifdef DBG_PRINT
				kprintf("Found queue entry to replace at position %d\n", i);
			#endif

			iter->data = newData;
			return OK;
		}
		i++;
		iter = iter->next;
	}while(iter != queueTail);

	#ifdef DBG_PRINT
		kprintf("Could not find data %d in the queue, no replacement performed!\n", toReplace);
	#endif

	restore(ps);
	return SYSERR;
}

void fq_clear(f_q_entry_t *queueHead, f_q_entry_t *queueTail)
{
	STATWORD ps;
	disable(ps);

	#ifdef DBG_PRINT
		kprintf("Clearing fifo queue\n");
	#endif

	int i = 0;
	f_q_entry_t *iter = queueHead;
	f_q_entry_t *next = iter;
	do
	{
		#ifdef DBG_PRINT
			kprintf("%d'th entry data: %d\taddress: 0x%08X\n", i, iter->data, iter);
		#endif

		next = iter->next;
		freemem((struct mblock *)iter, sizeof(f_q_entry_t));
		iter = next;
		i++;
	}while(iter != queueTail);

	freemem((struct mblock *)iter, sizeof(f_q_entry_t)); //free the queue tail

	#ifdef DBG_PRINT
		kprintf("Fifo queue is cleared!\n");
	#endif
	
	restore(ps);
}

void fq_print(f_q_entry_t *queueHead, f_q_entry_t *queueTail)
{
	STATWORD ps;
	disable(ps);

	f_q_entry_t *iter = queueHead;
	int i = 0;
	do
	{
		kprintf("%d'th entry data: %d\taddress: 0x%08X\n", i, iter->data, iter);
		iter = iter->next;
		i++;
	}while(iter != queueTail);

	restore(ps);
}
