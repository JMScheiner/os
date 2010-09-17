
#include <syscall_int.h>

.globl get_ticks

get_ticks: 
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	//Get tid off of stack.
	int 		$GET_TICKS_INT 	//Trap to get_ticks
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.

