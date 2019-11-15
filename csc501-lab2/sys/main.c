#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>


#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	1

void proc1_test1(char *msg, int lck) {
	char *addr;
	int i;

	get_bs(TEST1_BS, 100);

	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}

	addr = (char*) PROC1_VADDR;
	for (i = 0; i < 26; i++) {
		*(addr + i * NBPG) = 'A' + i;
	}

	sleep(6);

	for (i = 0; i < 26; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	xmunmap(PROC1_VPNO);
	return;
}

void proc1_test2(char *msg, int lck) {
	int *x;
	int *y;
	int *z;
	int *w;

	kprintf("ready to allocate x heap space\n");
	x = vgetmem(1024);
	kprintf("x heap allocated at %x\n", x);
	*x = 100;
	*(x + 1) = 200;
	kprintf("heap variable x: %d %d\n", *x, *(x + 1));

	kprintf("allocating y!\n");
	y = vgetmem(1024);
	kprintf("y allocated at %x\n", y);
	*y = 300;
	*(y + 1) = 400;
	kprintf("heap variable y: %d %d\n", *y, *(y + 1));

	kprintf("allocting z!\n");
	z = vgetmem(4);
	kprintf("z allocated at %x\n", z);
	*z = 20;
	kprintf("heap variable z: %d\n", *z);
	

	vfreemem(y, 1024);
	vfreemem(x, 1024);
	vfreemem(z, 4000);
}

void proc1_test3(char *msg, int lck) {

	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 0; i < 1024; i++) {
//		*(addr + i * NBPG) = 'B'; //This overwrites some kernel code, causes the whole thing to crash
	}

	for (i = 0; i < 1024; i++) {
		kprintf("%d\t0x%08x: %c\n", i, addr + i * NBPG, *(addr + i * NBPG));
	}
	return;
}

int main() {
	int pid1;
	int pid2;

	srpolicy(AGING);

	kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(10);


	kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);


	kprintf("\n3: Frame test\n");
	pid1 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
	resume(pid1);
	sleep(3);


	fq_clear(fq_head, fq_tail);
	cq_clear(cq_queueRoot);

}
