
#include <syscall_int.h>

.globl gettid

gettid: 
	int 		$GETTID_INT			//Trap to gettid
	ret								//Value is already in %eax, so return.

