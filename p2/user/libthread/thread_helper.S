
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

/** @def switch_stacks_and_vanish(tcb_t *tcb, char *stack_addr)
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
	movl   8(%ebp), %esp    // Move the new stack pointer into esp
	pushl  4(%ebp)          // Push the tcb as an argument
	call   clean_up_thread  // Clean up our stack frame and vanish

