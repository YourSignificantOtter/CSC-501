#ifndef _LOCK_H_
#define _LOCK_H_

/*
  ============================================================
	General Defines and Macros
  ============================================================
*/
//#define DBG_PRINT				// Controls debug printing
#define NLOCKS	50				// Number of locks available in the lock table
#define isbadlock(l)	(l < 0 || l >= NLOCKS)	// Is the passed lock id bad?
#define ppriopinh(p)	(proctab[p].pinh == 0 ? proctab[p].pprio : proctab[p].pinh)	//Choose pinh or pprio depending on pinh != 0

/*
  ============================================================
	Lock Status / User lock access types
  ============================================================
*/
#define FREE	0		// Lock is free for user to take
#define READ	FREE  + 1	// Lock is in read state/user attempting to use lock as a reader
#define WRITE	READ  + 1	// Lock is in write state/user attempting to use lock as a writer
#define DELETED	-6		// Lock has been deleted (-6 to avoid redefinition)

/*
  ============================================================
	Lock Type Define
  ============================================================
*/
typedef struct {
	int  		status; 		// Current lock status, as defined above
	int  		owner;			// The PID that owns the lock
	Bool 		pid[NPROC];		// Processes waiting on the lock
	int  		prio[NPROC];		// Lock priority of the processes waiting on the lock
	int  		accType[NPROC];		// Type of access that the user is waiting on
	unsigned long 	timeStamp[NPROC];	// Time the process was entered into the queue
} lock_t;

extern lock_t locktab[NLOCKS];	// The Lock table
extern unsigned long ctr1000;	// Counter in ms

/*
  ============================================================
	Function Declerations
  ============================================================
*/
int lock(int lkId, int accessType, int lkPrio);		// Function users call to attempt to gain a lock
void linit(void);					// Initialize the lock table
int lcreate(void);					// Users creating a lock, returns a lock descriptor
int releaseall(int numlk, long args);			// Releases numlk number of lock ids

int priorityInheritance(int lkId);			// Performs the priority inheritance tasks
void printLock(int lkId);				// Prints the locks information

#endif
