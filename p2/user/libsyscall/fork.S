
#include <syscall_int.h>

.globl fork 

fork: 
	int 	$FORK_INT		//Trap to fork
	ret						//%eax contains return value.


