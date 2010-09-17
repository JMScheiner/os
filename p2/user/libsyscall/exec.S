
#include <syscall_int.h>

.globl exec

print:
	pushl		%esi					//%esi is callee saved.	

	leal 		8(%esp),		%esi	//Load the address of the first argument.
	int 		$EXEC_INT			//Trap to exec.
	
	popl		%esi					//Pull the old %esi off the stack.
	ret 								//If we're still here, %eax contains the return value.


