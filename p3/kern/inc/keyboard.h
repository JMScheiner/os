
#ifndef KEYBOARD_UM5LT9N0

#define KEYBOARD_UM5LT9N0

//NOTE: This value must be a power of 2
//	512 is maybe excessively large for a key buffer, but
//	it is still relatively tiny.  If at some point we are
//	hurting for memory, this can be made 256 safely.
#define KEY_BUF_SIZE 512 

int readchar(void);

/** 
* @brief Wrapper for the keyboard ISR.
*/
void asm_keyboard_wrapper(void);

int readline(char *buf, int len);
void keyboard_init(void);

#endif /* end of include guard: KEYBOARD_UM5LT9N0 */


