
#ifndef THREAD_HELPER_H
#define THREAD_HELPER_H

/** @def get_addr(void)
 *
 * @brief Return the address of the stack pointer
 *
 * @return The current value of esp, the stack pointer
 */
char *get_addr(void);

/** @def switch_to_stack(char *stack_addr)
 *
 * @brief Replace the current stack with a new stack beginning at the specified
 *        address.
 *
 * @param stack_addr The address of the stack to switch to.
 */
void switch_to_stack(char *stack_addr);

#endif

