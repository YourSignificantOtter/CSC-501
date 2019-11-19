#ifndef _LOCK_H_
#define _LOCK_H_

#define DBG_PRINT					/* Control if the system should Print out
							 debug statements			*/

#define NLOCK 50					/* Number of R/W Locks			*/

#define	DELETED	-6					/* Lock has been deleted		*/
#define INIT	0					/* Lock is initialized but unused	*/
#define FREE	1					/* Lock is created but is not locked	*/
#define READ	2					/* Lock is for reading purposes		*/
#define WRITE	3					/* Lock is for writing purposes		*/

int prio_inherit(int pid, int lock);			/* Perform priority inheritance with proc
							pid attempting to gain lock		*/

int linit(void);					/* Initialize all the system R/W Locks	*/
int lcreate(void); 					/* Create a lock, returns an ID that can 
							be used to refer to the lock. or SYSERR
							when no more locks available		*/

int ldelete(int lockdescriptor);			/* Deletes the associated lock		*/
int lock(int ldes1, int type, int priority);		/* Obtains a lock for read/write 	*/
int releaseall(int numlocks, int args, ...);		/* Simulataneous release of numlocks	*/
void print_lock(int lock);				/* Print a locks info			*/

typedef struct {					/* Queue node type			*/
	int prio;					/* priority of the entry		*/
	int type;					/* READ/WRITE of the entry		*/
	int pid;					/* PID of the entry			*/
	struct q_node_t *next;				/* Pointer to the next node		*/
	struct q_node_t *prev;				/* Pointer to the previous node		*/
} q_node_t;

int q_enqueue(int prio, int type, int pid, int lock);	/* Emplace data into a lock's queue	*/
int q_dequeue(int prio, int type, int pid, int lock);	/* Remove data from a lock's queue	*/
void q_print(int lock);					/* Print a locks queue			*/

typedef struct {					/* Lock Type				*/
	int		status;				/* Lock Status				*/
	int		pcount;				/* Number of processes in queue		*/
//	int		currprio;			/* Priority of the process with lock	*/
	long		currpids;			/* PID(s) of the process with the lock	*/
	q_node_t	*head;				/* Head of the process queue		*/
	q_node_t	*tail;				/* Tail of the process queue		*/
} lock_t;

extern lock_t locks[];					/* Table of locks			*/

#define isbadlock(s)	(s < 0 || s >= NLOCK)		/* Macro to test if a lock id is bad	*/

#endif
