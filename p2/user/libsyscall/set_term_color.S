
#include <syscall_int.h>

.globl set_term_color

set_term_color:
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi		//Get color off the stack.
	int 		$SET_TERM_COLOR_INT 	//Trap to set_term_color
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.

