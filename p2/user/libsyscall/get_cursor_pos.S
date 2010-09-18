
#include <syscall_int.h>

.globl get_cursor_pos

get_cursor_pos:
	pushl		%esi					//%esi is callee saved.	

	leal 		8(%esp),		%esi		//Load the address of the first argument.
	int 		$GET_CURSOR_POS_INT	//Trap to get_cursor_pos.
	
	popl		%esi					//Pull the old %esi off the stack.
	ret 								//%eax already contains the return value.


