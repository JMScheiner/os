
.globl mode_switch

/** @def void mode_switch(void *stack_pointer, unsigned int eflags, 
 *                        void *instruction_pointer);
 *
 * @brief Switch to user mode.
 *
 * @param stack_pointer The top of the user stack
 *
 * @param eflags The user's eflags register
 *
 * @param instruction_pointer The next instruction the user will run.
 */
mode_switch:
	pushl   %ebp
	movl    %esp, %ebp
	movl    $SEGSEL_USER_DS, %ds  // Install data segment selectors into
	movl    $SEGSEL_USER_DS, %es  // all data segments.
	movl    $SEGSEL_USER_DS, %fs
	movl    $SEGSEL_USER_DS, %gs
	pushl   $SEGSEL_USER_DS       // Push user stack segment selector
	pushl   8(%ebp)               // Push user esp
	pushl   12(%ebp)              // Push user eflags
	pushl   $SEGSEL_USER_CS       // Push user code segment selector
	pushl   16(%ebp)              // Push user instruction pointer
	iret                          // return to user mode

