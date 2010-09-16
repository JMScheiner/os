
#include <syscall_int.h>

.globl set_status

set_status: 
	movl 	4(%esp), 	%esi	
	int 	$SET_STATUS_INT

