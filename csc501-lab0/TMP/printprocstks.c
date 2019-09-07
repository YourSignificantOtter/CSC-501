#include <kernel.h>
#include <proc.h>
#include <stdio.h>

static unsigned long *esp;

void printprocstks(int priority)
{
	kprintf("void printprocstks(int priority)\n");

	struct pentry *proc;
	int i = 0;
	for(;i < NPROC; i++) //Loop through all processes
	{
		proc = &proctab[i];

		if(isbadpid(i) || proc->pstate == PRFREE) //Check if this is actually a valid non-free process
			continue; //Skip it if it isnt

		if(proc->pprio > priority) //print only procs with higher prio
		{
			kprintf("Process: [%s]\n", proc->pname);
			kprintf("\tpid: %d\n", i);
			kprintf("\tpriority: %d", proc->pprio);
			kprintf("\tbase: %08X\n", proc->pbase);
			kprintf("\tlimit: %08X\n", proc->plimit);
			kprintf("\tlen: %d\n", proc->pstklen);
			//Check if we are looking at the current process
			if(currpid == i)
			{
				//Need the stack pointer for this process, not the saved stack pointer
				asm("movl %esp, esp");
				kprintf("\tpointer: %08X\n", esp);
			}
			else
				kprintf("\tpointer: %08X\n", proc->pesp);
		}	
	}

	kprintf("\n");
}
