/** @file asm_helper.h
 *
 * @brief Helper assembly functions
 *
 * @author Tim Wilson
 * @author Justin Scheiner
 */

#ifndef ASM_HELPER_BGUWQ67UHG
#define ASM_HELPER_BGUWQ67UHG

/** @def void *get_esp(void)
 *
 * @brief Get current stack pointer.
 *
 * @return The current stack pointer
 */
void *get_esp();

/** @def void halt(void)
 *
 * @brief Halt the system
 */
void halt();

#endif
