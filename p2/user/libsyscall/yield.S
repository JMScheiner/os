
#include <syscall_int.h>

.globl yield

yield: 
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	//Get tid off of stack.
	int 		$YIELD_INT 			//Trap to yield
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.

