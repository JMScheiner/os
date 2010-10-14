
#include <x86/cr.h>

.globl context_switch

/** @def void context_switch(void **old_esp, void *new_esp)
 * 
 * @brief Switch from executing in the current kernel thread to executing
 *        in another kernel thread.
 *
 * Stack    * Offset
 *******************
 * %edi     * 0
 * %esi     * 4
 * %ebp     * 8
 * %ebx     * 12
 * %edx     * 16
 * %ecx     * 20
 * %eax     * 24
 * %eip     * 28
 * old_esp  * 32
 * new_esp  * 36
 *
 * @param old_esp Address to store the stack pointer of the current thread
 *
 * @param new_esp Stack address to jump execution to.
 */
context_switch:
	call   get_cr3         // Save the page directory
	pusha                  // Save general purpose registers
	movl   32(%esp), %eax  // Save stack pointer so someone else 
	movl   %esp, (%eax)    // can context switch back to us
	movl   36(%esp), %esp  // Switch to someone else
	popa                   // Restore general purpose registers
	pushl  %eax            // Set page directory as argument
	call   set_cr3         // Restore the page directory
	addl   $4, %esp        // Pop page directory off stack
	ret

