/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{

	setschedclass(2);
	int test = getschedclass();
	kprintf("%d\n", test);
	return 0;
}
