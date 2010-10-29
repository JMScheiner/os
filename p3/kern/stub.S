
.globl pop_stub
.globl loop_stub

pop_stub: 
	popa        /* Restore user registers */
	iret        /* Return from interrupt */

loop_stub: 
   jmp loop_stub

