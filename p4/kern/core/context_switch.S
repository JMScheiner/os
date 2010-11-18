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
 * %cr3     * 0
 * %edi     * 4
 * %esi     * 8
 * %ebp     * 12
 * %esp     * 16
 * %ebx     * 20
 * %edx     * 24
 * %ecx     * 28
 * %eax     * 32
 * %eip     * 36
 * old_esp  * 40
 * new_esp  * 44
 * pd       * 48
 *
 * @param old_esp Address to store the stack pointer of the current thread
 * @param new_esp Stack address to jump execution to.
 * @param pd The page directory of the kernel thread we are jumping to.
 */
context_switch:
   pusha                  // Save general purpose registers
   movl   %cr2,     %eax  // Push %cr2 onto the stack.
   pushl  %eax
   movl   40(%esp), %eax  // Save stack pointer so someone else 
   movl   %esp,    (%eax) // can context switch back to us
   movl   44(%esp), %eax  // Switch to someone else
   movl   48(%esp), %ebx  // Grab the page directory
   movl   %ebx,     %cr3  // Switch page directories. 
   movl   (%eax),   %esp  // Jump stacks.
   popl   %eax
   movl   %eax,     %cr2  // Restore %cr2
   popa                   // Restore general purpose registers
   ret

