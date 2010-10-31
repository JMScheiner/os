
#ifndef KEYBOARD_UM5LT9N0
#define KEYBOARD_UM5LT9N0

#include <reg.h>

//NOTE: This value must be a power of 2
//	2048 is maybe excessively large for a key buffer, but
//	it matches the size of the console.
#define KEY_BUF_SIZE 2048

int readchar(void);

/** 
* @brief Wrapper for the keyboard ISR.
*/
void asm_keyboard_wrapper(void);

void getchar_handler(volatile regstate_t reg);
void readline_handler(volatile regstate_t reg);
int readline(char *buf, int len);
void keyboard_init(void);

#endif /* end of include guard: KEYBOARD_UM5LT9N0 */


