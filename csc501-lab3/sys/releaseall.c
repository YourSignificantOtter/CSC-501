#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

/* -------------------------------------------------------
 * lreleaseall - simultaneous release of numlocks
 * variadic function that takes inputs as integers on the 
 * stack. ints are lock ids
 * -------------------------------------------------------
 */
int releaseall(int numlocks, int args, ...)
{

}
