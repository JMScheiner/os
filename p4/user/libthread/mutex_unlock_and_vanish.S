
.globl mutex_unlock_and_vanish

#include <syscall_int.h>

mutex_unlock_and_vanish:
   movl        4(%esp), %eax     // Grab the mutex.
   movl        $1, %ecx            // Set %ecx to 1 for exchange and add.
   movl        $-1, 8(%eax)      // Set the active_tid to -1.
   lock xaddl  %ecx, 4(%eax)      // Add 1 to "now_serving," unlocking the kill stack.
   movl        8(%esp), %esp     // Jump to the "int" stack.
   INT         $VANISH_INT       // Trap to vanish on the int stack.

