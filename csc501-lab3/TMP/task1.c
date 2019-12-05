#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

#define LOCK_PRIO 	20
#define LOW_PRIO	5
#define MED_PRIO	10
#define HIGH_PRIO	20
/*
int mystrncmp(char* des,char* target,int n)
{
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}
*/
char testOutput[4];
int testOutputIdx;
Bool medBlock;

void lowPrio(char *msg, int lck, int sem)
{
//	#ifdef DBG_PRINT
		kprintf("Low Prio process started!\n");
		kprintf("Acquiring %s...", msg);
//	#endif

	int lk = -1;

	if(mystrncmp(msg, "lck", 3) == 0)
		lk = lock(lck, WRITE, LOCK_PRIO);
	else
		wait(sem);

//	#ifdef DBG_PRINT
		kprintf("Low Prio process acquired %s, sleeping for 1 sec\n", msg);
//	#endif

	sleep(1);

	//The low prio process is blocking the high prio one, but the med should NOT block this one
	testOutput[testOutputIdx++] = 'L'; //L for Low

//	#ifdef DBG_PRINT
		kprintf("Low Prio process releasing %s\n", msg);
//	#endif

	if(lk != -1)
		releaseall(1, lck); //release the lock
	else if(sem != -1)
		signal(sem); //release the semaphore
}

void medPrio(char *msg, int lck, int sem)
{
//	#ifdef DBG_PRINT
		kprintf("Med Prio process started!\n");
//	#endif

	sleep(5);
	medBlock = TRUE;
	testOutput[testOutputIdx++] = 'M'; //M for Med
	sleep(2);
}

void highPrio(char *msg, int lck, int sem)
{
//	#ifdef DBG_PRINT
		kprintf("High Prio process started!\n");
		kprintf("Acquiring %s...", msg);
//	#endif

	int lk = -1;

	if(mystrncmp(msg, "lck", 3) == 0)
		lk = lock(lck, WRITE, LOCK_PRIO);
	else
		wait(sem);

//	#ifdef DBG_PRINT
		kprintf("High Prio process acquired %s\n", msg);
//	#endif

	if(medBlock == TRUE)
	{
//		#ifdef DBG_PRINT
			kprintf("The medium priority process blocked the high priority process!\n");
//		#endif
	}
	//The low prio process is blocking the high prio one, but the med should NOT block this one
	testOutput[testOutputIdx++] = 'H'; //H for High

//	#ifdef DBG_PRINT
		kprintf("High Prio process releasing %s\n", msg);
//	#endif

	if(lk != -1)
		releaseall(1, lck); //release the lock
	else if(sem != -1)
		signal(sem); //release the semaphore
}

/*
============================================================
	task1 - this task compares the read/write locks
	created in this project with the semaphores that
	are part of XINU. Looks to see how each one handles
	priority inversion
============================================================
*/
void task1()
{
//	#ifdef DBG_PRINT
		kprintf("Task 1 running!\n");
		kprintf("General Idea is to have 3 processes\n");
		kprintf("Proc 1 is low prio, proc 2 is med, proc 3 is high\n");
		kprintf("Proc 1 will acquire the lock/sem and proc 3 will block on it\n");
		kprintf("Proc 2 will then come in and try to block Proc 1, thus also blocking Proc 3\n");
//	#endif

	int lk_proc1, lk_proc2, lk_proc3;
	int sem_proc1, sem_proc2, sem_proc3;

	medBlock = FALSE; //Medium process sets this to TRUE, should be low when high prio acquires the lock
	testOutputIdx = 0;
	testOutput[3] = '\0';

//	#ifdef DBG_PRINT
		kprintf("Starting with read/write lock approach\n");
//	#endif

	int lck = lcreate(); //create the lock
	int sem = screate(2); //create the semaphore

	lk_proc1 = create(lowPrio, 2000, LOW_PRIO, "Low Priority", 3, "lck", lck, sem);
	lk_proc2 = create(medPrio, 2000, MED_PRIO, "Med Priority", 3, "lck", lck, sem); 
	lk_proc3 = create(highPrio, 2000, HIGH_PRIO, "High Priority", 3, "lck", lck, sem);
	
	resume(lk_proc1);
	//sleep 1 second to let proc 1 get the lock
	sleep(1);

	//the high priority process will now be blocked by the low priority one
	resume(lk_proc3);

	//The med prio should NOT block the low prio proc
	resume(lk_proc2);

	sleep(10);

//	#ifdef DBG_PRINT
		kprintf("R/W Locks testOutput: %s\n", testOutput);
		kprintf("Now testing semaphores!\n\n\n");
//	#endif

	//Re-init values
	medBlock = FALSE; //Medium process sets this to TRUE, should be low when high prio acquires the lock
	testOutputIdx = 0;
	testOutput[3] = '\0';

	sem_proc1 = create(lowPrio, 2000, LOW_PRIO, "Low Priority", 3, "sem", lck, sem);
	sem_proc2 = create(medPrio, 2000, MED_PRIO, "Med Priority", 3, "sem", lck, sem); 
	sem_proc3 = create(highPrio, 2000, HIGH_PRIO, "High Priority", 3, "sem", lck, sem);

	resume(sem_proc1);
	//sleep 1 second to let proc 1 get the semaphore
	sleep(1);

	//the high priority process will now be blocked by the low priority one
	resume(sem_proc3);

	//The med prio should NOT block the low prio proc
	resume(sem_proc2);

	sleep(10);

//	#ifdef DBG_PRINT
		kprintf("semaphore testOutput: %s\n", testOutput);
		kprintf("Task1 Done!\n\n");
//	#endif
}
