
#include <syscall_int.h>

.globl misbehave

misbehave: 
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	//Get misbehave mode off of stack.
	int 		$MISBEHAVE_INT		//Trap to set status
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.

