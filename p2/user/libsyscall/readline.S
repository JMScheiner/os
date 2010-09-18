
#include <syscall_int.h>

.globl readline

readline:
	pushl		%esi					//%esi is callee saved.	

	leal 		8(%esp),		%esi	//Load the address of the first argument.
	int 		$READLINE_INT		//Trap to readline.
	
	popl		%esi					//Pull the old %esi off the stack.
	ret 								//If we're still here, %eax contains the return value.


