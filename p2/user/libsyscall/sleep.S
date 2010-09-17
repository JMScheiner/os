
#include <syscall_int.h>

.globl sleep

sleep:
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	//Get ticks off the stack.
	int 		$SLEEP_INT 			//Trap to sleep
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.

