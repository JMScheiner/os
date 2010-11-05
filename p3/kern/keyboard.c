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
#include <console.h>
#include <ecodes.h>

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
static unsigned int keybuf_divider = 0;
static unsigned int keybuf_tail = 0;
static int newlines = 0;

static mutex_t keyboard_lock;

static cond_t keyboard_signal;

/** @brief Get the index in keybuf following the given index. */
#define NEXT(index) \
	(((index) + 1) & (KEY_BUF_SIZE - 1))

/** @brief Get the index in keybuf preceding the given index. */
#define PREV(index) \
	(((index) - 1) & (KEY_BUF_SIZE - 1))

void getchar_handler(volatile regstate_t reg)
{
	lprintf("Ignoring getchar");
	MAGIC_BREAK;
   //TODO
}

/** @brief Reads the next line from the console and copies it into the
 * buffer pointed to by buf. 
 *
 * If there is no line of input currently available, the calling thread 
 * is descheduled until one is. If some other thread is descheduled on a 
 * readline() or a getchar(), then the calling thread must block and 
 * wait its turn to access the input stream. The length of the buffer 
 * is indicated by len. If the line is smaller than the buffer, then the 
 * complete line including the newline character is copied into the 
 * buffer. If the length of the line exceeds the length of the buffer, 
 * only len characters should be copied into buf. Available characters 
 * should not be committed into buf until there is a newline character 
 * available, so the user has a chance to backspace over typing mistakes. 
 *
 * Characters that will be consumed by a readline() should be echoed to 
 * the console as soon as possible. If there is no outstanding call to 
 * readline() no characters should be echoed. Echoed user input may be 
 * interleaved with output due to calls to print(). Characters not
 * placed in the buffer should remain available for other calls to
 * readline() and/or getchar(). Some kernel implementations may choose to
 * regard characters which have been echoed to the screen but which have
 * not been placed into a user buffer to be "dedicated" to readline() and
 * not available to getchar(). 
 *
 * The readline system call returns the number of bytes copied into the 
 * buffer. An integer error code less than zero is returned if buf is 
 * not a valid memory address, if buf falls in a read-only memory region 
 * of the task, or if len is "unreasonably" large.
 *
 * @param reg The register state on entry containing the buffer to read to
 * and the maximum length to read.
 */
void readline_handler(volatile regstate_t reg)
{
	char *arg_addr = (char *)SYSCALL_ARG(reg);
   int len;
   char* buf;
   
   if(v_memcpy((char*)&len, arg_addr, sizeof(int)) < sizeof(int))
   {
      debug_print("readline", "arg len unreadable.");
      RETURN(SYSCALL_INVALID_ARGS);
   }
   
   if(v_memcpy((char*)&buf, arg_addr + sizeof(int), sizeof(char*)) < 
			 sizeof(char*))
   {
      debug_print("readline", "arg buf unreadable.");
      RETURN(SYSCALL_INVALID_ARGS);
   }
   
	if (len < 0 || len > KEY_BUF_SIZE) {
      debug_print("readline", "len %d unreasonable.", len);
		RETURN(READLINE_INVALID_LENGTH);
	}
   
   /* FIXME We copy a string here - and so should use v_strcpy to avoid
    *  a race condition with remove_pages
    * */
	if (!mm_validate_write(buf, len)) {
      debug_print("readline", "buf unwritable.");
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
		if (c == '\b') {
			if (keybuf_tail != keybuf_head && 
					keybuf_tail != keybuf_divider) {
				keybuf_tail = PREV(keybuf_tail);
				putbyte(c);
			}
		}
		else {
			keybuf[keybuf_tail] = c;
			keybuf_tail = next_tail;
			putbyte(c);
		}

		/* A blocked thread can be released if a full line has been read, 
		 * or if more characters have been read than are currently being 
		 * waited for. */
		if (c == '\n') {
			newlines++;
			keybuf_divider = keybuf_tail;
			cond_signal(&keyboard_signal);
		}
	}
	outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

int readline(char *buf, int len) {
	/* Prevent other readers from interfering. They can't accomplish 
	 * anything until we're done anyway. */
	mutex_lock(&keyboard_lock);
	
	quick_lock();
	if (newlines == 0) {
		/* Wait for the keyboard_handler to process a full line. */
		cond_wait(&keyboard_signal);
	}
	else {
		quick_unlock();
	}
	int read;
	assert(newlines > 0);
   
	debug_print("readline", "Beginning read!");
	for (read = 0; read < len; read++) {
		buf[read] = keybuf[keybuf_head];
		keybuf_head = NEXT(keybuf_head);
		if (buf[read] == '\n') {
			atomic_add(&newlines, -1);
			break;
		}
	}
	debug_print("readline", "Read complete!");
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


