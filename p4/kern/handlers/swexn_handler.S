/** 
* @file swexn_handler.S
* @brief Assembly handler to return to a software exception handler.
* @author Tim Wilson
* @author Justin Scheiner
*/

.globl swexn_return

/** @def void swexn_return(void *eip, unsigned int cs_reg, 
 *               unsigned int eflags, void *esp, unsigned int ss_reg)
 * 
 * @brief Return to user mode in a software interrupt handler.
 *
 * Stack    * Offset
 *******************
 * ret_addr * 0
 * eip      * 4
 * cs_reg   * 8
 * eflags   * 12
 * esp      * 16
 * ss_reg   * 20
 *
 * @param eip The first instruction in the exception handler.
 * @param cs_reg The user cs register.
 * @param eflags The eflags register at the time the exception occurred.
 * @param esp The top address of the exception handler's stack.
 * @param ss_reg The user ss register.
 */
swexn_return:
   addl  $4, %esp     // Ignore the return address.
   xor   %eax, %eax   // Blank out the general purpose registers
   xor   %ebx, %ebx
   xor   %ecx, %ecx
   xor   %edx, %edx
   xor   %esi, %esi
   xor   %edi, %edi
   xor   %ebp, %ebp
   iret               // The arguments form the iret frame

