

#include <syscall_int.h>

.globl getchar

getchar: 
	int 		$GETCHAR_INT		//Trap to getchar
	ret								//Value is already in %eax, so return.
										//Do we need to worry about it being only the bottom byte?

