
.globl get_addr

/** @def get_addr(void)
 *
 * @brief Return the address of the stack pointer
 *
 * @return The current value of esp, the stack pointer
 */
get_addr:
   movl   %esp, %eax      // Move the current stack pointer to eax
   ret                    // Return the address of the stack pointer

.globl switch_stacks_and_vanish

/** @def switch_stacks_and_vanish(int tid, char *old_stack, char *new_stack)
 *
 * @brief Replace the current stack with a new stack beginning at the specified
 *        address and dispose of ourself.
 *
 * @param tcb Out thread control block
 *
 * @param stack_addr The address of the stack to switch to.
 */
switch_stacks_and_vanish:
   movl   %esp, %ebp       // Save the stack pointer
   movl   12(%ebp), %esp    // Move the new stack pointer into esp
   pushl  8(%ebp)          // Push the old_stack as an argument
   pushl  4(%ebp)          // Push the tid as an argument
   call   clean_up_thread  // Clean up our stack frame and vanish

