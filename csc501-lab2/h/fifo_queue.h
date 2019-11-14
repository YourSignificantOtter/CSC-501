#ifndef _FIFO_QUEUE_H_
#define _FIFO_QUEUE_H_

typedef struct {
	int data; //Data stored within the queue node
	struct f_q_entry_t *prev; //Pointer to the previous entry of the queue
	struct f_q_entry_t *next; //Pointer to the next entry of the queue
} f_q_entry_t;

void init_fifo_queue(f_q_entry_t *queueHead, f_q_entry_t *queueTail);
int fq_enqueue(int data, f_q_entry_t *queueHead, f_q_entry_t *queueTail); //Push a new element into the queue
int fq_dequeue(int data, f_q_entry_t *queueHead, f_q_entry_t *queueTail); //Remove an element from the queue by data
int fq_replace(int toReplace, int newData, f_q_entry_t *queueHead, f_q_entry_t *queueTail); //Replace an element in the queue by data
void fq_print(f_q_entry_t *queueHead, f_q_entry_t *queueTail); //Print the queue data

#endif
