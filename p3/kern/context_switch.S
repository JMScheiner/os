
.globl context_switch

/** @def void context_switch(void **old_esp, void *new_esp)
 * 
 * @brief Switch from executing in the current thread to executing
 *        in another thread.
 *
 * @param old_esp Address to store the stack pointer of the current thread
 *
 * @param new_esp Stack address to jump execution to.
 */
context_switch:
	pushl  %ebp
	movl   %esp, %ebp
	call   get_cr3         // Save the page directory
	pushl  %cs             // Save code segment
	pushl  %ds             // Save data segment
	pushf                  // Save eflags
	pusha                  // Save general purpose registers
	
	// Save stack pointer so eventually someone can context switch back to us
	movl   %esp, 8(%ebp)

	// Switch to someone else
	movl   12(%ebp), %esp
	popa                   // Restore general purpose registers
	popf                   // Restore eflags
	popl   %ds             // Restore data segment
	popl   %cs             // Restore code segment
	movl   %ds, %ss        // Restore stack segment
	movl   %ds, %es        // Restore other data segments
	movl   %ds, %fs
	movl   %ds, %gs
	pushl  %eax
	call   set_cr3         // Restore the page directory
	leave
	ret

