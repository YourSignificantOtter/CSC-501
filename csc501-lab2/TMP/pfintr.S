/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:
	cli //disable interrupts
	popl pferrcode // get error code
	pushfl //push flags
	pushal //push regs
	call pfint //call c function
	popal //pop regs
	popfl //pop flags
	iret //return

