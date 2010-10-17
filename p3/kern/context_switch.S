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

/** 
* @brief Creates a new thread context identical to the current one. 
*
*  Invokes memcpy, copying the stack part of the kernel stack region.
*
*  After this function returns, it is safe to add the new thread to the
*     scheduler queue, and it will begin running with an updated TCB, 
*     and with the same stack context / register state.
*
*  It should be safe to context switch while this is happening.
* 
* @param old_bottom The bottom of the stack we are executing on. 
* @param new_bottom The bottom of the stack we plan on adding to the scheduler. 
*/
duplicate_thread_context: 
	call   get_cr3         // Save the current page directory
   pusha 
   movl   36(%esp), %eax  // Grab the bottom of the current kernel stack. 
   movl   40(%esp), %ebx  // Grab the bottom of the new kernel stack. 
   subl   %esp,     %eax  // Calculate the depth of the current stack. 
   subl   %eax,     %ebx  // Calculate the %esp for the new stack. 
   pushl  %eax            // Prepare for memcpy, n
   pushl  %esp            // Prepare for memcpy, source. 
   pushl  %ebx            // Prepare for memcpy, target.
   call   memcpy          // Make the call. 
   popl   %eax            // Pop arguments.
   popl   %eax
   popl   %ebx
   movl   44(%esp), %eax  // Grab the address of the saved %esp
   movl   %ebx,   (%eax)  // Copy over the new stack pointer.    
   popa
   ret 


duplicate_proc_context: 
   mov    16(%esp),  %eax  // Save the new page directory.
   pusha 
   movl   36(%esp), %eax  // Grab the bottom of the current kernel stack. 
   movl   40(%esp), %ebx  // Grab the bottom of the new kernel stack. 
   subl   %esp,     %eax  // Calculate the depth of the current stack. 
   subl   %eax,     %ebx  // Calculate the %esp for the new stack. 
   pushl  %eax            // Prepare for memcpy, n
   pushl  %esp            // Prepare for memcpy, source. 
   pushl  %ebx            // Prepare for memcpy, target.
   call   memcpy          // Make the call. 
   popl   %eax            // Pop arguments.
   popl   %eax
   popl   %ebx
   movl   44(%esp), %eax  // Grab the address of the saved %esp
   movl   %ebx,   (%eax)  // Copy over the new stack pointer.    
   popa
   ret 

