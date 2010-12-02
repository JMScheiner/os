/** 
* @file keyboard.h
* @brief Responsible for responding to keyboard interrupts
*  and generally for the keyboard interface.
*
* @author Justin Scheiner
* @author Tim Wilson
*/

#ifndef KEYBOARD_UM5LT9N0
#define KEYBOARD_UM5LT9N0

#include <reg.h>
#include <ureg.h>
#include <types.h>

/* NOTE: This value should be a power of 2, since we do mod by & */
#define KEY_BUF_SIZE 2048

int readchar(void);

/** 
* @brief Wrapper for the keyboard ISR.
*/
void asm_keyboard_wrapper(void);

void echo_to_console();
void getchar_handler(ureg_t*  reg);
void readline_handler(ureg_t*  reg);
int readline(char *buf, int len);
void keyboard_init(void);

#endif /* end of include guard: KEYBOARD_UM5LT9N0 */


