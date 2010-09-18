#include <syscall_int.h>

.globl halt

halt: 
	int 	$HALT_INT		//Trap to halt
	ret



