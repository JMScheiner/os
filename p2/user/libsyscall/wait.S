
#include <syscall_int.h>

.globl wait

wait: 
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	//Get status_ptr off the stack.
	int 		$WAIT_INT			//Trap to wait
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.


