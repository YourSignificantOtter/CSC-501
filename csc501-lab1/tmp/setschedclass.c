#include <kernel.h>
#include <stdio.h>
#include "lab1.h"

int currScheduleType;// = EXPDISTSCHED;

void setschedclass(int sched_class)
{
	if(sched_class != EXPDISTSCHED && sched_class != LINUXSCHED)
		kprintf("Bad scheduler type, no changes will be made!\n");
	else
		currScheduleType = sched_class;
}
