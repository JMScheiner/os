
#include <syscall_int.h>

.globl set_status

set_status: 
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	//Get status off of stack.
	int 		$SET_STATUS_INT	//Trap to set status
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.

