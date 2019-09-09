#include <kernel.h>
#include <stdio.h>

int currScheduleType = EXPDISTSCHED;

void setschedclass(int sched_class)
{
	if(sched_class != EXPDISTSCHED && sched_class != LINUXSCHED)
	{
		kprintf("Bad scheduler type input, defaulting to EXPDISTSCHED!\n");
		currScheduleType = EXPDISTSCHED;
	}
	else
		currScheduleType = sched_class;
}

int getschedclass()
{
	return currScheduleType;
}
