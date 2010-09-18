
#include <syscall_int.h>

.globl get_ticks

get_ticks: 
	int 		$GET_TICKS_INT 	//Trap to get_ticks
	ret								//Value is already in %eax, so return.

