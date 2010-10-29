
.globl pop_stub
.globl loop_stub

pop_stub: 
	popa        /* Restore user registers */
	iret        /* Return from interrupt */

loop_stub: 
   sti
loop_forever:
   jmp loop_forever 

