#ifndef _CIRCULAR_QUEUE_H_
#define _CIRCULAR_QUEUE_H_

typedef struct {
	int data; //Data store within the queue node
	struct c_q_entry_t *prev; //Pointer to the previous entry of the queue
	struct c_q_entry_t *next; //Pointer to the next entry of the queue
} c_q_entry_t;

void init_circular_queue(c_q_entry_t *queueRoot);
int cq_enqueue(int data, c_q_entry_t *queueRoot); //Push new element into the queue
int cq_dequeue(int data, c_q_entry_t *queueRoot); //Remove an element from the queue by data
int cq_replace(int toReplace, int newData, c_q_entry_t *queueRoot); //Replace an element from the queue by data
void cq_print(c_q_entry_t *queueRoot); //Print the queue data

#endif
