
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
	pushl  %ds             // Save data segment
	pushf                  // Save eflags
	pusha                  // Save general purpose registers
	//pushw  %cs
	movl   8(%ebp), %ecx
	// Save stack pointer so eventually someone can context switch back to us
	movl   %esp, (%ecx)
	// Switch to someone else
	movl   12(%ebp), %esp
	//popw   %cx
	//ljmp   %cx, $cs_reload
//cs_reload:
	popa                   // Restore general purpose registers
	popf                   // Restore eflags
	popl   %ds             // Restore data segment
	mov    %ds, %ecx
	movw   %cx, %ss        // Restore stack segment
	movw   %cx, %es        // Restore other data segments
	movw   %cx, %fs
	movw   %cx, %gs
	pushl  %eax
	call   set_cr3         // Restore the page directory
	leave
	ret

