#include <kernel.h>
#include <stdio.h>
#include "lab0.h" //Get the #defines for sys call info

int sysCallCounters[NPROC][NSYS];
unsigned long sysCallExecTimes[NPROC][NSYS];
Bool sysCallCounterFlag = FALSE;
const char *sysCallNames[NSYS]; //= {FREEMEM_STR, CHPRIO_STR, GETPID_STR, GETPRIO_STR, GETTIME_STR, KILL_STR, RECEIVE_STR, RECVCLR_STR, RECVTIM_STR, RESUME_STR, SCOUNT_STR, SDELETE_STR, SEND_STR, SETDEV_STR, SETNOK_STR, SIGNALLN_STR, SLEEP_STR, SLEEP10_STR, SLEEP100_STR, SLEEP1000_STR, SRESET_STR, STACKTRACE_STR, SUSPEND_STR, UNSLEEP_STR, WAIT_STR};


void generateSysCallNames() //There is 100% a better way to do this but the above array initialization did not work 
{
	int sysNum = 0;
	for(; sysNum < NSYS; sysNum++)
	{
		switch(sysNum)
		{
			case FREEMEM_NUM:
				sysCallNames[sysNum] = FREEMEM_STR;
				break;
			case CHPRIO_NUM:
				sysCallNames[sysNum] = CHPRIO_STR;
				break;
			case GETPID_NUM:
				sysCallNames[sysNum] = GETPID_STR;
				break;
			case GETPRIO_NUM:
				sysCallNames[sysNum] = GETPRIO_STR;
				break;
			case GETTIME_NUM:
				sysCallNames[sysNum] = GETTIME_STR;
				break;
			case KILL_NUM:
				sysCallNames[sysNum] = KILL_STR;
				break;
			case RECEIVE_NUM:
				sysCallNames[sysNum] = RECEIVE_STR;
				break;
			case RECVCLR_NUM:
				sysCallNames[sysNum] = RECVCLR_STR;
				break;
			case RECVTIM_NUM:
				sysCallNames[sysNum] = RECVTIM_STR;
				break;
			case RESUME_NUM:
				sysCallNames[sysNum] = RESUME_STR;
				break;
			case SCOUNT_NUM:
				sysCallNames[sysNum] = SCOUNT_STR;
				break;
			case SDELETE_NUM:
				sysCallNames[sysNum] = SDELETE_STR;
				break;
			case SEND_NUM:
				sysCallNames[sysNum] = SEND_STR;
				break;
			case SETDEV_NUM:
				sysCallNames[sysNum] = SETDEV_STR;
				break;
			case SETNOK_NUM:
				sysCallNames[sysNum] = SETNOK_STR;
				break;
			case SCREATE_NUM:
				sysCallNames[sysNum] = SCREATE_STR;
				break;
			case SIGNAL_NUM:
				sysCallNames[sysNum] = SIGNAL_STR;
				break;
			case SIGNALLN_NUM:
				sysCallNames[sysNum] = SIGNALLN_STR;
				break;
			case SLEEP_NUM:
				sysCallNames[sysNum] = SLEEP_STR;
				break;
			case SLEEP10_NUM:
				sysCallNames[sysNum] = SLEEP10_STR;
				break;
			case SLEEP100_NUM:
				sysCallNames[sysNum] = SLEEP100_STR;
				break;
			case SLEEP1000_NUM:
				sysCallNames[sysNum] = SLEEP1000_STR;
				break;
			case SRESET_NUM:
				sysCallNames[sysNum] = SRESET_STR;
				break;
			case STACKTRACE_NUM:
				sysCallNames[sysNum] = STACKTRACE_STR;
				break;
			case SUSPEND_NUM:
				sysCallNames[sysNum] = SUSPEND_STR;
				break;
			case UNSLEEP_NUM:
				sysCallNames[sysNum] = UNSLEEP_STR;
				break;
			case WAIT_NUM:
				sysCallNames[sysNum] = WAIT_STR;
				break;
			default:
				break;
		}
	}
}

void syscallsummary_start()
{
	//Clear the current counters to 0
	int i = 0, j = 0;
	for(; i < NPROC; i++)
	{
		for(; j < NSYS; j++)
		{
			sysCallCounters[i][j] = 0;
			sysCallExecTimes[i][j] = 0;
		}
	}
	//Begin tracking the system calls
	sysCallCounterFlag = TRUE;	
}

void syscallsummary_stop()
{
	//Stop tracking the system calls
	sysCallCounterFlag = FALSE;
}

void printsyscallsummary()
{
	kprintf("void printsyscallsummary()\n");
	generateSysCallNames();
	int pid = 0;
	int sysNum = 0;
	Bool printPID = TRUE;
	for(; pid < NPROC; pid++)
	{
		printPID = TRUE;
		for(sysNum = 0; sysNum < NSYS; sysNum++)
		{
			if(sysCallCounters[pid][sysNum] != 0)
			{
				if(printPID == TRUE)
				{
					kprintf("Process [pid:%d]\n", pid);
					printPID = FALSE;
				}

				kprintf("\tSyscall: %s, count: %d, average execution time: %08d (ms)\n", sysCallNames[sysNum], sysCallCounters[pid][sysNum], sysCallExecTimes[pid][sysNum]/sysCallCounters[pid][sysNum]);
			}
		}
	}


}
