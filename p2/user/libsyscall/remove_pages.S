
#include <syscall_int.h>

.globl remove_pages

remove_pages:
	pushl		%esi					//%esi is callee saved.	
	
	movl 		8(%esp), 	%esi	//Get base off the stack.
	int 		$REMOVE_PAGES_INT //Trap to remove_pages
	
	popl		%esi					//Pull the old %esi off the stack.
	ret								//Value is already in %eax, so return.

