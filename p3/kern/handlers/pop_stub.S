
.globl pop_stub

pop_stub: 
	popa        /* Restore user registers */
	iret        /* Return from interrupt */

