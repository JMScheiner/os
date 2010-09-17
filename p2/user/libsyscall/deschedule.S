
#include <syscall_int.h>

.globl deschedule

deschedule: 
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	//Get reject* off of stack.
	int 		$DESCHEDULE_INT	//Trap to deschedule
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.

