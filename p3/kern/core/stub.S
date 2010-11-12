
.globl pop_stub
.globl loop_stub

/** @def void pop_stub()
 *
 * @brief "Return" to the user for the first time after creating their
 * process from fork.
 */
pop_stub: 
   cli                   /* Disable interrupts */
   call set_esp0_helper  /* Set esp0 so we return to the right kernel stack. */
   popa                  /* Restore user registers */
   iret                  /* Return from interrupt */

/** @def void loop_forever()
 *
 * @brief Loop forever, waiting for a device like the timer or keyboard
 * handler to release somebody else to run.
 */
loop_stub: 
   sti               /* Make sure interrupts are enabled. */
loop_forever:
   jmp loop_forever  /* Do nothing */

