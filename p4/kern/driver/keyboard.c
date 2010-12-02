/** 
* @file keyboard.c
* @brief Responsible for responding to keyboard interrupts
*  and generally for the keyboard interface.
*
* @author Justin Scheiner
* @author Tim Wilson
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
#include <types.h>

/*********************************************************************/
/*                                                                   */
/* Keyboard driver interface                                         */
/*                                                                   */
/*********************************************************************/

/*
 * Simple ring buffer: The key buffer is divided logically into two parts
 * by a divider. Characters before the divider are promised to readers.
 * The keyboard handler cannot overwrite them with backspaces. The
 * characters after the divider are forbidden to readers and can be freely
 * overwritten by backspaces.
 *
 * When the key buffer becomes full, repeatedly overwrite the last
 * character in the buffer.
 */

/** @brief The buffer for characters. */
static char keybuf[KEY_BUF_SIZE];

/** @brief The first unread character in the buffer. */
static unsigned int keybuf_head = 0;

/** @brief The last character in the buffer that a backspace can delete
 * because it has not been promised to a reader. */
static unsigned int keybuf_divider = 0;

/** @brief One past the last character in the buffer. */
static unsigned int keybuf_tail = 0;

/*
 * The print keybuf stores characters that should be printed to the
 * console screen. The keyboard handler cannot print these characters,
 * because printing requires locking the print lock. Therefore readers
 * must take care of printing characters from the buffer that the handler
 * will populate.
 */

/** @brief The buffer for characters to be printed to the screen. Includes
 * backspace characters. */
static char print_keybuf[KEY_BUF_SIZE];

/** @brief The first unread character in the print buffer. */
static unsigned int print_keybuf_head = 0;

/** @brief One past the last character in the print buffer. */
static unsigned int print_keybuf_tail = 0;

/** @brief Mutual exclusion lock ensuring reads from the keyboard are not
 * interleaved. */
static mutex_t keyboard_lock;

/** @brief A signal used to notify a reader when a full line is available.
 */
static cond_t keyboard_signal;

/** @brief True iff there is a reader waiting for a line. */
static boolean_t reader = FALSE;

/** @brief True iff a full line has been printed to the console, but not
 * yet completely consumed by readlines. */
static boolean_t full_line = FALSE;

/** @brief Get the index in keybuf following the given index. */
#define NEXT(index) \
   (((index) + 1) & (KEY_BUF_SIZE - 1))

/** @brief Get the index in keybuf preceding the given index. */
#define PREV(index) \
   (((index) - 1) & (KEY_BUF_SIZE - 1))

static inline void async_putbyte(char c);

/** 
* @brief Returns a single character from the character input stream. 
* If the input stream is empty the thread is descheduled until 
*  a character is available. If some other thread is descheduled 
*  on a readline() or getchar(), then the calling thread must block
*  and wait its turn to access the input stream. Characters processe
* 
* @param reg The register state on entry to the handler.
*/
void getchar_handler(ureg_t* reg)
{
   debug_print("keyboard", "Ignoring getchar");
   RETURN(reg, EFAIL);
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
void readline_handler(ureg_t* reg)
{
   char *arg_addr = (char *)SYSCALL_ARG(reg);
   int len;
   char* buf;
   char readbuf[KEY_BUF_SIZE];
   
   if(v_copy_in_int(&len, arg_addr) < 0)
      RETURN(reg, EARGS);
   
   if(v_copy_in_ptr(&buf, arg_addr + sizeof(int)) < 0)
      RETURN(reg, EARGS);
   
   if (len < 0 || len > KEY_BUF_SIZE) {
      debug_print("readline", "len %d unreasonable.", len);
      RETURN(reg, ELEN);
   }
   
   debug_print("readline", "0x%x: reading up to %d chars to %p\n", 
         get_tcb()->tid, len, buf);
   
   int read = readline(readbuf, len);
   int copied;
   if ((copied = v_memcpy(buf, readbuf, read, FALSE)) != read) {
      debug_print("readline", "Only wrote %d out of %d chars read", copied, read);
      RETURN(reg, EBUF);
   }
   RETURN(reg, read);
}

/**
 * @brief Place a character in the buffer of characters to be printed to
 * the screen. They will not be printed until a reader arrives who will
 * read them.
 *
 * @param c The character to place in the buffer.
 */
static inline void async_putbyte(char c) {
   int next = NEXT(print_keybuf_tail);
   if (next != print_keybuf_head) {
      print_keybuf[print_keybuf_tail] = c;
      print_keybuf_tail = next;
   }
}

/**
 * @brief Echo the characters in the key buffer that will be read by
 * readline to the console screen.
 */
void echo_to_console() {
   if (reader && !full_line) {
      mutex_t *lock = get_print_lock();
      mutex_lock(lock);
      while (print_keybuf_head != print_keybuf_tail) {
         char c = print_keybuf[print_keybuf_head];
         putbyte(c);
         print_keybuf_head = NEXT(print_keybuf_head);
         if (c == '\n') {
            reader = FALSE;
            full_line = TRUE;
            /* Notify any readers */
            cond_signal(&keyboard_signal);
            break;
         }
      }
      mutex_unlock(lock);
   }
}

/** 
* @brief Process a scancode from the keyboard port. If there is space
* available, store it in the keybuf queue.
*
* If a character is read that can unblock a thread waiting for a line, 
* do so.
*/
void keyboard_handler(void)
{
   int next_tail = NEXT(keybuf_tail);
   kh_type augchar = process_scancode(inb(KEYBOARD_PORT));
   if (KH_HASDATA(augchar) && KH_ISMAKE(augchar)) {
      char c = KH_GETCHAR(augchar);

      if (c == '\b') {
         if (keybuf_tail != keybuf_head && 
               keybuf_tail != keybuf_divider) {
            keybuf_tail = PREV(keybuf_tail);
            async_putbyte(c);
         }
      }
      else {
         if (next_tail == keybuf_head && 
               keybuf[PREV(keybuf_tail)] != '\n') {
            // Backup one char so we can place the new char
            next_tail = keybuf_tail;
            keybuf_tail = PREV(keybuf_tail);
            async_putbyte('\b');
         }
         if (next_tail != keybuf_head) {
            keybuf[keybuf_tail] = c;
            keybuf_tail = next_tail;
            async_putbyte(c);
            if (c == '\n') {
               /* A blocked thread can be released if a full line has 
                * been read, so move up the keybuf_divider. */
               keybuf_divider = keybuf_tail;
            }
         }
      }
   }
   outb(INT_CTL_PORT, INT_ACK_CURRENT);
  
   /* Echo characters to the screen if there is a reader waiting. */
   enable_interrupts();
   echo_to_console();
}

/**
 * @brief Read a line entered from the keyboard into buf. This
 * runs serially.
 *
 * @param buf The buffer to read into. This should be a safe buffer in
 * kernel memory.
 * @param len The maximum number of bytes to read into the buffer.
 *
 * @return The number of characters read into the buffer.
 */
int readline(char *buf, int len) {
   mutex_lock(&keyboard_lock);

   /* Indicate there is a reader so we echo to the console. */
   reader = TRUE;

   /* Wait until a full line has been placed in the buffer. */
   quick_lock();
   if (!full_line)
      cond_wait(&keyboard_signal);
   else
      quick_unlock();

   assert(full_line);
   int read;
   debug_print("readline", "Beginning read!");
   for (read = 0; read < len; read++) {
      buf[read] = keybuf[keybuf_head];
      keybuf_head = NEXT(keybuf_head);
      if (buf[read] == '\n') {
         full_line = FALSE;
         read++;
         break;
      }
   }
   debug_print("readline", "Read complete!");
   mutex_unlock(&keyboard_lock);
   return read;
}

/** 
* @brief Initialize keyboard driver.
*/
void keyboard_init(void)
{
   mutex_init(&keyboard_lock);
   cond_init(&keyboard_signal);
}


