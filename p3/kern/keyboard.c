/** 
* @file keyboard.c
* @brief Responsible for responding to keyboard interrupts
* 	and generally for the keyboard interface.
*
* @author Justin Scheiner
* @bug Definitely not thread safe!!!
*
*/

#include <keyboard.h>
#include <interrupt_defines.h>
#include <keyhelp.h>
#include <asm.h>
#include <stdint.h>
#include <simics.h>
#include <cond.h>
#include <mutex.h>
#include <atomic.h>
#include <mm.h>
#include <debug.h>
#include <thread.h>
#include <vstring.h>

#define READLINE_INVALID_ARGS -1
#define READLINE_INVALID_LENGTH -2
#define READLINE_INVALID_BUFFER -3

/*********************************************************************/
/*                                                                   */
/* Keyboard driver interface                                         */
/*                                                                   */
/*********************************************************************/

/*
 * Simple ring buffer: 
 */
static char keybuf[KEY_BUF_SIZE];
static unsigned int keybuf_head = 0;
static unsigned int keybuf_tail = 0;
static int newlines = 0;
static int line_length = KEY_BUF_SIZE;

static mutex_t keyboard_lock;

static cond_t keyboard_signal;

/** @brief Get the index in keybuf following the given index. */
#define NEXT(index) \
	(((index) + 1) & (KEY_BUF_SIZE - 1))

/** @brief Get the number of keys currently in the key buffer. */
#define NUM_KEYS \
	((keybuf_tail - keybuf_head + KEY_BUF_SIZE) & (KEY_BUF_SIZE - 1))

void getchar_handler(volatile regstate_t reg)
{
	lprintf("Ignoring getchar");
	MAGIC_BREAK;
   //TODO
}

void readline_handler(volatile regstate_t reg)
{
	char *arg_addr = (char *)SYSCALL_ARG(reg);
   int len;
   char* buf;
   
   if(v_memcpy((char*)&len, arg_addr, sizeof(int)) < sizeof(int))
      RETURN(READLINE_INVALID_ARGS);
   
   if(v_memcpy((char*)&buf, arg_addr + sizeof(int), sizeof(char*)) < sizeof(char*))
      RETURN(READLINE_INVALID_ARGS);
   
	if (len < 0 || len > KEY_BUF_SIZE) {
		RETURN(READLINE_INVALID_LENGTH);
	}
   
   /* FIXME We copy a string here - and so should use v_strcpy to avoid
    *  a race condition with remove_pages
    * */
	if (!mm_validate_write(buf, len)) {
		RETURN(READLINE_INVALID_BUFFER);
	}

	debug_print("readline", "0x%x: reading up to %d chars to %p\n", 
			get_tcb()->tid, len, buf);

	int read = readline(buf, len);
	RETURN(read);
}


/** @brief Returns the next character in the keyboard buffer
 *
 *  This function does not block if there are no characters in the keyboard
 *  buffer
 *
 *  @return The next character in the keyboard buffer, or -1 if the keyboard
 *          buffer is currently empty
 **/
/*
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
*/



/** 
* @brief Process a scancode from the keyboard port. If there is space
* available, store it in the keybuf queue.
*
* If a character is read that can unblock a thread waiting for a line, do so.
*/
void keyboard_handler(void)
{
	int next_tail = NEXT(keybuf_tail);
	kh_type augchar = process_scancode(inb(KEYBOARD_PORT));
	if (next_tail != keybuf_head && 
			KH_HASDATA(augchar) && 
			KH_ISMAKE(augchar)) {
		char c = KH_GETCHAR(augchar);
		keybuf[keybuf_tail] = c;
		keybuf_tail = next_tail;

		/* A blocked thread can be released if a full line has been read, or if
		 * more characters have been read than are currently being waited for. */
		if (c == '\n') {
			newlines++;
			cond_signal(&keyboard_signal);
		}
		else if (NUM_KEYS >= line_length) {
			cond_signal(&keyboard_signal);
		}
	}
	outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

int readline(char *buf, int len) {
	/* Prevent other readers from interfering. They can't accomplish anything
	 * until we're done anyway. */
	mutex_lock(&keyboard_lock);
	line_length = len;

	disable_interrupts();
	if (newlines == 0 && NUM_KEYS < len) {
		/* Wait for the keyboard_handler to process a full line. */
		cond_wait(&keyboard_signal);
	}
	enable_interrupts();
	int read;
	for (read = 0; read < len; read++) {
		buf[read] = keybuf[keybuf_head];
		keybuf_head = NEXT(keybuf_head);
		if (buf[read] == '\n') {
			atomic_add(&newlines, -1);
			break;
		}
	}
	mutex_unlock(&keyboard_lock);
	return read;
}

/** 
* @brief Initialize keyboard driver.
* 	(Really does nothing for now).
*/
void keyboard_init(void)
{
	mutex_init(&keyboard_lock);
	cond_init(&keyboard_signal);
}


