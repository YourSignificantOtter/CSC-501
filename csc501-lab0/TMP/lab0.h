#ifndef _LAB0_H_
#define _LAB0_H_

//zfunction
long zfunction(long param);

//printsegaddress
void printsegaddress();

//printtos
void printtos();

//printprocstks
void printprocstks(int priority);

//prinsyscallsummary
void printsyscallsummary();
void syscallsummary_start();
void syscallsummary_stop();

//define the syscall numbers
#define FREEMEM_NUM	 0
#define CHPRIO_NUM	 1
#define GETPID_NUM	 2
#define GETPRIO_NUM	 3
#define GETTIME_NUM	 4
#define KILL_NUM	 5
#define RECEIVE_NUM	 6
#define RECVCLR_NUM	 7
#define RECVTIM_NUM	 8
#define RESUME_NUM	 9
#define SCOUNT_NUM	10
#define SDELETE_NUM	11
#define SEND_NUM	12
#define SETDEV_NUM	13
#define SETNOK_NUM	14
#define SCREATE_NUM	15
#define SIGNAL_NUM	16
#define SIGNALLN_NUM	17
#define SLEEP_NUM	18
#define SLEEP10_NUM	19
#define SLEEP100_NUM	20
#define SLEEP1000_NUM	21
#define SRESET_NUM	22
#define STACKTRACE_NUM	23
#define SUSPEND_NUM	24
#define UNSLEEP_NUM	25
#define WAIT_NUM	26

//Define the syscall strings
#define FREEMEM_STR	"sys_freemem"
#define CHPRIO_STR	"sys_chprio"
#define GETPID_STR	"sys_getpid"
#define GETPRIO_STR	"sys_getprio"
#define GETTIME_STR	"sys_gettime"
#define KILL_STR	"sys_kill"
#define RECEIVE_STR	"sys_receive"
#define RECVCLR_STR	"sys_recvclr"
#define RECVTIM_STR	"sys_recvtim"
#define RESUME_STR	"sys_resume"
#define SCOUNT_STR	"sys_scount"
#define SDELETE_STR	"sys_sdelete"
#define SEND_STR	"sys_send"
#define SETDEV_STR	"sys_setdev"
#define SETNOK_STR	"sys_setnok"
#define SCREATE_STR	"sys_screate"
#define SIGNAL_STR	"sys_signal"
#define SIGNALLN_STR	"sys_signalln"
#define SLEEP_STR	"sys_sleep"
#define SLEEP10_STR	"sys_sleep10"
#define SLEEP100_STR	"sys_sleep100"
#define SLEEP1000_STR	"sys_sleep1000"
#define SRESET_STR	"sys_sreset"
#define STACKTRACE_STR	"sys_stacktrace"
#define SUSPEND_STR	"sys_suspend"
#define UNSLEEP_STR	"sys_unsleep"
#define WAIT_STR	"sys_wait"

extern const char *sysCallNames[NSYS];// = {FREEMEM_STR, CHPRIO_STR, GETPID_STR, GETPRIO_STR, GETTIME_STR, KILL_STR, RECEIVE_STR, RECVCLR_STR, RECVTIM_STR, RESUME_STR, SCOUNT_STR, SDELETE_STR, SEND_STR, SETDEV_STR, SETNOK_STR, SIGNALLN_STR, SLEEP_STR, SLEEP10_STR, SLEEP100_STR, SLEEP1000_STR, SRESET_STR, STACKTRACE_STR, SUSPEND_STR, UNSLEEP_STR, WAIT_STR};
extern unsigned long ctr1000;

#endif
