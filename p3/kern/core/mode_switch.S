/** @file mode_switch.S
 *
 * @brief mode_switch functionality for getting the first process to
 * user mode.
 *
 * @author Tim Wilson
 * @author Justin Scheiner
 */
#include <x86/seg.h>

.globl mode_switch

/** @def void mode_switch(void *kernel_stack, void *user_stack, 
 *                        unsigned int eflags, void *instruction_pointer);
 *
 * @brief Switch to user mode.
 *
 * @param kernel_stack The part of the kernel stack to return to.
 *
 * @param user_stack The top of the user stack
 *
 * @param eflags The user's eflags register
 *        Bit 1 (reserved) should be set
 *        Bit 9 (IF Interrupt Enable) should be set
 *        Bits 12,13 (IOPL IO Privilege Level) should not be set
 *
 * @param instruction_pointer The next instruction the user will run.
 */
mode_switch:
   pushl   %ebp
   movl    %esp, %ebp
   pushl   8(%ebp)
   call    set_esp0              // Set return point of kernel stack
   addl    $4, %esp
   pushl   $SEGSEL_USER_DS       // Push user stack segment selector
   pushl   12(%ebp)              // Push user esp
   pushl   16(%ebp)              // Push user eflags
   pushl   $SEGSEL_USER_CS       // Push user code segment selector
   pushl   20(%ebp)              // Push user instruction pointer
   movl    $SEGSEL_USER_DS, %eax
   movw    %ax, %ds              // Install data segment selectors into
   movw    %ax, %es              // all data segments.
   movw    %ax, %fs
   movw    %ax, %gs
   xor     %eax, %eax            // Clear general purpose registers
   xor     %ebx, %ebx            // for user
   xor     %ecx, %ecx
   xor     %edx, %edx
   xor     %esi, %esi
   xor     %edi, %edi
   xor     %ebp, %ebp
   iret                          // return to user mode

