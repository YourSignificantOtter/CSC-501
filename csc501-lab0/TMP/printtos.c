#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

static unsigned long *esp;
static unsigned long *ebp;

void printtos()
{
	//After getting into printtos
	asm("movl %esp, esp");

	//Before getting into printtos
	asm("movl %ebp, ebp");

	kprintf("void printtos()\n");

	int test = 0xFFFF0000;
	int test2 = 0x0000FFFF;
	int test3 = 0xDEADBEEF;
	int test4 = 0x5555AAAA; //Testing Local Variables

	kprintf("Before[0x%08X]: 0x%08X\n", ebp + 2, *(ebp + 2));
	kprintf("After [0x%08X]: 0x%08X\n", esp, *esp);

	//Print up to 4 elements
	int i = 0;
	for(; i < 4; i++)
	{
		esp--;
		if(*esp == MAGIC || esp == ebp)
			break;
		else
		{
			kprintf("\telement[0x%08X]: 0x%08X\n", esp, *esp);
		}
		
	}
	kprintf("\n");
}
