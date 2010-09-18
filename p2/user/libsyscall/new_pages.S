
#include <syscall_int.h>

.globl new_pages

new_pages:
	pushl		%esi					//%esi is callee saved.	

	leal 		8(%esp),		%esi	//Load the address of the first argument.
	int 		$NEW_PAGE_INT		//Trap to new_pages.
	
	popl		%esi					//Pull the old %esi off the stack.
	ret 								//If we're still here, %eax contains the return value.


