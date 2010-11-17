/** @file asm_helper.S
 *
 * @brief Helper assembly functions
 *
 * @author Tim Wilson
 * @author Justin Scheiner
 */

.globl get_esp
.globl halt

/** @def void *get_esp(void)
 *
 * @brief Get current stack pointer.
 *
 * @return The current stack pointer
 */
get_esp:
   movl  %esp, %eax
   ret

/** @def void halt(void)
 *
 * @brief Halt the system
 */
halt: 
   HLT
