#include <syscall_int.h>

.globl make_runnable

make_runnable: 
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	 	//Get tid off of stack.
	int 		$MAKE_RUNNABLE_INT	//Trap to make_runnable
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.

