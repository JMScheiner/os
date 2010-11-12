/** 
* @file context_switch.S
* @brief Context switching code for the 410 kernel.
* @author Tim Wilson
* @author Justin Scheiner
*/

#include <x86/cr.h>

.globl context_switch

/** @def void context_switch(void **old_esp, void **new_esp, void* pd)
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
 * pd       * 44
 *
 * @param old_esp Address to store the stack pointer of the current thread
 * @param new_esp Stack address to jump execution to.
 * @param pd The page directory of the kernel thread we are jumping to.
 */
context_switch:
	pusha                  // Save general purpose registers
	movl   36(%esp), %eax  // Save stack pointer so someone else 
	movl   %esp, (%eax)    //   can context switch back to us
	movl   40(%esp), %eax  // Switch to someone else
	movl   44(%esp), %ebx  // Grab the page directory
	movl   %ebx,     %cr3  // Switch page directories. 
	movl   (%eax),   %esp  // Jump stacks.
	popa                   // Restore general purpose registers
	ret

