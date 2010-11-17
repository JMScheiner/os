/** 
* @file mode_switch.h
*
* @brief Defintion for mode_switch
*  - which takes us into user space for the first time. 
*
* @author Tim Wilson
* @author Justin Scheiner
* @date 2010-11-12
*/

#ifndef MODE_SWITCH_HGF87HGF3
#define MODE_SWITCH_HGF87HGF3

/** @brief Switch to user mode.
 *
 * @param kernel_stack The part of the kernel stack to return to.
 *
 * @param user_stack The top of the user stack
 *
 * @param eflags The user's eflags register
 *        Bit 1 (reserved) should be set
 *        Bit 9 (IF Interrupt Enable) should be set
 *        Bits 12,13 (IOPL IO Privilege Level) should be set
 *        Bit 18 (AC Alignment Checking) should be unset
 *
 * @param instruction_pointer The next instruction the user will run.
 */
void mode_switch(void *kernel_stack, void *user_stack, 
                 unsigned int eflags, void *instruction_pointer);

#endif

