/** 
* @file keyboard.c
* @brief Responsible for responding to keyboard interrupts
* 	and generally for the keyboard interface.
*
* @author Justin Scheiner
* @bug Definitely not thread safe!!!
*
*/

#include "keyboard.h"
#include <interrupt_defines.h>
#include <keyhelp.h>
#include <asm.h>
#include <stdint.h>
#include <simics.h>
#include <scheduler.h>

//NOTE: This value must be a power of 2
//	512 is maybe excessively large for a key buffer, but
//	it is still relatively tiny.  If at some point we are
//	hurting for memory, this can be made 256 safely.
#define KEY_BUF_SIZE 512 

/*********************************************************************/
/*                                                                   */
/* Keyboard driver interface                                         */
/*                                                                   */
/*********************************************************************/

/*
 * Simple ring buffer: 
 * 	If tail = head, the buffer is empty. Otherwise: 
 * 		You can always write to keybuf[tail]
 * 		You can always read from keybuf[head]
 *		readchar cannot r/w keybuf[tail]
 *		interrupt cannot r/w keybuf[head] (unless head = tail)
 */
uint8_t keybuf[KEY_BUF_SIZE];
unsigned int keybuf_head = 0;
unsigned int keybuf_tail = 0;

/** @brief Returns the next character in the keyboard buffer
 *
 *  This function does not block if there are no characters in the keyboard
 *  buffer
 *
 *  @return The next character in the keyboard buffer, or -1 if the keyboard
 *          buffer is currently empty
 **/
int readchar(void)
{
	kh_type augchar;
	while(keybuf_head != keybuf_tail)
	{
		augchar = process_scancode(keybuf[keybuf_head]);
		keybuf_head = (keybuf_head + 1) & (KEY_BUF_SIZE - 1);
		
		if(KH_HASDATA(augchar) && KH_ISMAKE(augchar))
			return KH_GETCHAR(augchar);
	}
	return -1;
}

/** 
* @brief Buffers a scancode for processing later.
*/
void keyboard_handler(void)
{
	keybuf[keybuf_tail++] = inb(KEYBOARD_PORT);
	keybuf_tail = keybuf_tail & (KEY_BUF_SIZE - 1);

	//Explicitly start dropping keys if we need to.
	//	The oldest keypresse gets lost first.
	if(keybuf_tail == keybuf_head)
	{
		keybuf_head++;
		keybuf_head = keybuf_head & (KEY_BUF_SIZE - 1);
	}

	outb(INT_CTL_PORT, INT_ACK_CURRENT);
   
   /* FIXME Call the scheduler in the keyboard handler. */
   scheduler_next();

}

/** 
* @brief Initialize keyboard driver.
* 	(Really does nothing for now).
*/
void keyboard_init(void)
{
}


