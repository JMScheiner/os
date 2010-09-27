.globl mutex_unlock_and_vanish

#include <syscall_int.h>

mutex_unlock_and_vanish:
	movl            4(%esp), %eax     //Grab the mutex.
	movl            $1, %ecx          //Set %ecx to one for exchange and add.
	lock xaddl      %ecx, 8(%eax)     //Add 1 to "now_serving" and set %ecx to its old value.
	INT $VANISH_INT
