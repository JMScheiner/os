
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

.globl switch_to_stack

/** @def switch_to_stack(char *stack_addr)
 *
 * @brief Replace the current stack with a new stack beginning at the specified
 *        address.
 *
 * @param stack_addr The address of the stack to switch to.
 */
switch_to_stack:
	movl   %esp, %ebp      // Save the stack pointer
	movl   8(%ebp), %esp   // Move the new stack pointer into esp
	pushl  4(%ebp)         // Push the address we will return to
	ret                    // Return

