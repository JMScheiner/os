
#include <syscall_int.h>

.globl vanish

vanish: 
	int 	$VANISH_INT	

