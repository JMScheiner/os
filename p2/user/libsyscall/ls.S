
#include <syscall_int.h>

.globl ls

ls:
	pushl		%esi					//%esi is callee saved.	

	leal 		8(%esp),		%esi	//Load the address of the first argument.
	int 		$LS_INT				//Trap to ls.
	
	popl		%esi					//Pull the old %esi off the stack.
	ret 								//%eax already contains the return value.


