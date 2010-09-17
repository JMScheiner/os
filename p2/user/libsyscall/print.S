
#include <syscall_int.h>

.globl print

print:
	pushl		%esi					//%esi is callee saved.	

	leal 		8(%esp),		%esi	//Load the address of the first argument.
	int 		$PRINT_INT			//Trap to print.
	
	popl		%esi					//Pull the old %esi off the stack.
	ret 								//%eax already contains the return value.


