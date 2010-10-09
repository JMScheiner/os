
#ifndef MODE_SWITCH_HGF87HGF3
#define MODE_SWITCH_HGF87HGF3

/** @brief Switch to user mode.
 *
 * @param stack_pointer The top of the user stack
 *
 * @param eflags The user's eflags register
 *
 * @param instruction_pointer The next instruction the user will run.
 */
void mode_switch(void *stack_pointer, unsigned int eflags, 
                 void *instruction_pointer);

#endif

