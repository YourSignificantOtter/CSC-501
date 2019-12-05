//PA3 addition this file added so we can remove reading processes from the ready queue when a writer takes the lock

#include <conf.h>
#include <kernel.h>
#include <q.h>

/*
  ============================================================
	remove - removes an item from the queue by key
		proc - the process to remove from the ready queue
		head - the head of the queue to remove from
		returns OK or SYSERR;
  ============================================================
*/
int remove(int proc, int head)
{
	int next;
	int prev;

	prev = q[proc].qprev;
	next = q[proc].qnext;

	q[proc].qkey = 0;
	q[prev].qnext = next;

	return OK;
}

int printq(int head)
{
	int iter;

	iter = q[head].qnext;
	while(q[iter].qkey != MAXINT)
	{
		kprintf("iter: %d\tkey: %d\n", iter, q[iter].qkey);
		iter = q[iter].qnext;
	}
}
