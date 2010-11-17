
#ifndef THREAD_HELPER_H
#define THREAD_HELPER_H

#include <thr_internals.h>

/** @def get_addr()
 *
 * @brief Return the address of the stack pointer
 *
 * @return The current value of esp, the stack pointer
 */
char *get_addr();

/** @def switch_stacks_and_vanish(int tid, char *old_stack, char *new_stack)
 *
 * @brief Replace the current stack with a new stack beginning at the specified
 *        address and dispose of ourself.
 *
 * @param tcb Out thread control block
 *
 * @param stack_addr The address of the stack to switch to.
 */
void switch_stacks_and_vanish(int tid, char *old_stack, char *new_stack);

#endif

