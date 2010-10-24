/** 
* @file context_switch.S
* @brief Context switching code for the 410 kernel.
* @author Tim Wilson
* @author Justin Scheiner
* @bug Care has to be taken as to when duplicate_thread_context is called. 
*  In particulare, local references to "my tcb" become inaccurate when we 
*  eventually context switch to the child. We should put more thought into
*  whether there is anything "bad" on the stack in the new childs thread.
* @date 2010-10-16
*/

#include <x86/cr.h>

.globl context_switch
.globl duplicate_thread_context
.globl duplicate_proc_context

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
 * %esp     * 12
 * %ebx     * 16
 * %edx     * 20
 * %ecx     * 24
 * %eax     * 28
 * %eip     * 32
 * old_esp  * 36
 * new_esp  * 40
 *
 * @param old_esp Address to store the stack pointer of the current thread
 *
 * @param new_esp Stack address to jump execution to.
 */
context_switch:
	call   get_cr3         // Save the page directory
	pusha                  // Save general purpose registers
	movl   36(%esp), %eax  // Save stack pointer so someone else 
	movl   %esp, (%eax)    // can context switch back to us
	movl   40(%esp), %eax  // Switch to someone else
	movl   (%eax), %esp    // 
	popa                   // Restore general purpose registers
	pushl  %eax            // Set page directory as argument
	call   set_cr3         // Restore the page directory
	addl   $4, %esp        // Pop page directory off stack
	ret

