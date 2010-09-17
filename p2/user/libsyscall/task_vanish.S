
#include <syscall_int.h>

.globl task_vanish

task_vanish: 
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	//Get status off the stack.
	int 		$TASK_VANISH_INT	//Trap to task_vanish
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//void



