
.globl get_esp

/** @def void *get_esp(void)
 *
 * @brief Get current stack pointer.
 *
 * @return The current stack pointer
 */
get_esp:
	movl  %esp, %eax
	ret

