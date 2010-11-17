
#include <syscall_int.h>

.globl main

main: 

   int $THREAD_FORK_INT
   testl  %eax, %eax 
   jnz PARENT
CHILD:
   jmp CHILD

PARENT: 
   jmp PARENT
   nop
   


