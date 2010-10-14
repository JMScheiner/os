
#ifndef KEYBOARD_UM5LT9N0

#define KEYBOARD_UM5LT9N0

int readchar(void);

/** 
* @brief Wrapper for the keyboard ISR.
*/
void asm_keyboard_wrapper(void);

void keyboard_init(void);

#endif /* end of include guard: KEYBOARD_UM5LT9N0 */


