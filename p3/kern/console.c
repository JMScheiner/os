
#include <console.h>
#include <video_defines.h>
#include <asm.h>
#include <keyboard.h>
#include <mm.h>
#include <simics.h>
#include <thread.h>
#include <debug.h>
#include <vstring.h>
#include <mutex.h>

#define CONSOLE_END ((char*)(CONSOLE_MEM_BASE + 2 * CONSOLE_WIDTH * CONSOLE_HEIGHT))
#define MAX_VALID_COLOR 0x8f
   
#define PRINT_INVALID_ARGS -1

/* This may seem rather large, but it is manageable, and actually the largest 
 *    amount of data that can be on the screen. */
#define PRINT_BUF_SIZE ((CONSOLE_WIDTH * CONSOLE_HEIGHT) + 1)

/***************** Console State:  ****************/
/* Invariant: Always points to the next position to write. */
static int console_color = FGND_WHITE | BGND_BLACK;
static int console_row = 0;
static int console_col = 0;
static int cursor_hidden = 1;

/* Print buffer protects us from printing from an invalid 
 *  user space buffer. */
static char printbuf[PRINT_BUF_SIZE];
mutex_t print_lock;

void console_init()
{
   mutex_init(&print_lock);
}


/** 
* @brief Prints len bytes of memory, starting at buf, to the console. 
*  The calling thread should block until all characters have been 
*     printed to the console. 
*  
*  Output of two concurrent print()s should not be intermixed. 
*  If len is larger than some reasonable maximum or if buf is not a 
*     valid memory address, an integer error code less than zero should be 
*     returned.
*  Characters printed to the console invoke standard newline, backspace, and scrolling behaviors.
* 
* @param reg The register state on entry to print.
*/
void print_handler(volatile regstate_t reg)
{
   /* The interface and code is very similar to readline 
    *    i.e. forgive copy and paste code. */
	char *arg_addr = (char *)SYSCALL_ARG(reg);
   int len;
   char* buf;
   
   if(v_memcpy((char*)&len, arg_addr, sizeof(int)) < sizeof(int))
      RETURN(PRINT_INVALID_ARGS);
   
   if(v_memcpy((char*)&buf, arg_addr + sizeof(int), sizeof(char*)) < sizeof(char*))
      RETURN(PRINT_INVALID_ARGS);

	if (len < 0 || len > PRINT_BUF_SIZE)
		RETURN(PRINT_INVALID_ARGS);

   /* Copy buffer. TODO We could potentially put a much smaller
    *    buffer on the stack, and not have to hold this lock through
    *    the copying process.  Alternatively, we can write a version
    *    of putbytes that does validated copying, but in that case 
    *    we would probably end up putting only part of the input on
    *    the console. 
    **/
   mutex_lock(&print_lock);
   if(v_memcpy((char*)printbuf, buf, len) != len)
   {
      mutex_unlock(&print_lock);
      RETURN(PRINT_INVALID_ARGS);
   }
   putbytes(printbuf, len);
   mutex_unlock(&print_lock);
   RETURN(0);
}

void set_term_color_handler(volatile regstate_t reg)
{
	lprintf("Ignoring set_term_color");
	MAGIC_BREAK;
   //TODO
}

void set_cursor_pos_handler(volatile regstate_t reg)
{
	lprintf("Ignoring set_cursor_pos");
	MAGIC_BREAK;
   //TODO
}

void get_cursor_pos_handler(volatile regstate_t reg)
{
	lprintf("Ignoring get_cursor_pos");
	MAGIC_BREAK;
   //TODO
}
/************** End Syscall wrappers . **************/



static void set_cursor_position(int row, int col)
{
	int address = row * CONSOLE_WIDTH + col;
	
	//Write LSB to the LSB index.
	outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
	outb(CRTC_DATA_REG, address & 0xff);
	
	//Write MSB to the MSB index.
	outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
	outb(CRTC_DATA_REG, (address >> 8) & 0xff);
}

/** 
* @brief Scrolls the console, writing the default console color
* 	to the new line.
*
* 	Does not modify cursor position.
*/
void scroll_console(void)
{
	char *out = (char*)CONSOLE_MEM_BASE;	
	char *in = (char*)(CONSOLE_MEM_BASE + 2 * CONSOLE_WIDTH);
	
	while(in < CONSOLE_END)
		*(out++) = *(in++);

	while(out < CONSOLE_END)
	{
		*(out++) = 0;
		*(out++) = console_color;
	}
}

/** @brief Prints character ch at the current location
 *         of the cursor.
 *
 *  If the character is a newline ('\n'), the cursor is
 *  be moved to the beginning of the next line (scrolling if necessary).
 *  If the character is a carriage return ('\r'), the cursor
 *  is immediately reset to the beginning of the current
 *  line, causing any future output to overwrite any existing
 *  output on the line.  If backspace ('\b') is encountered,
 *  the previous character is erased.  See the main console.c description
 *  for more backspace behavior.
 *
 *  @param ch the character to print
 *  @return The input character
 */
int putbyte(char ch)
{

	//TODO If any function here should be cleaned up, it's this one.
	switch(ch)
	{
		case '\n': 
			console_col = 0;
			console_row++;
			break;
		
		case '\r':
			console_col = 0;
			break;
		
		case '\b':
			if(console_col != 0) 
				console_col--;
			draw_char(console_row, console_col, ' ', console_color);
			break;
		
		default: 
			draw_char(console_row, console_col, ch, console_color);
			console_col++;
			break;
	}
	
	if(console_col >= CONSOLE_WIDTH)
	{
		console_col = 0;
		console_row++;
	}

	if(console_row >= CONSOLE_HEIGHT)
	{
		console_row = CONSOLE_HEIGHT - 1;
		scroll_console();
	}

	if(!cursor_hidden)
		set_cursor_position(console_row, console_col);

	return (int)(unsigned char)ch;
}

/** @brief Prints the string s, starting at the current
 *         location of the cursor.
 *
 *  If the string is longer than the current line, the
 *  string fills up the current line and then
 *  continues on the next line. If the string exceeds
 *  available space on the entire console, the screen
 *  scrolls up one line, and then the string
 *  continues on the new line.  If '\n', '\r', and '\b' are
 *  encountered within the string, they are handled
 *  as per putbyte. If len is not a positive integer or s
 *  is null, the function has no effect.
 *
 *  @param s The string to be printed.
 *  @param len The length of the string s.
 *  @return Void.
 */
void putbytes(const char* s, int len)
{
	if(s && len > 0)
		while(len--)
			putbyte(*(s++));
}

/** @brief Prints character ch with the specified color
 *         at position (row, col).
 *
 *  If any argument is invalid, the function has no effect.
 *
 *  @param row The row in which to display the character.
 *  @param col The column in which to display the character.
 *  @param ch The character to display.
 *  @param color The color to use to display the character.
 *  @return Void.
 */
void draw_char(int row, int col, int ch, int color)
{
	//TODO Should we validate 'ch' somehow?
	if(row < CONSOLE_HEIGHT && 
		col < CONSOLE_WIDTH && 
		color <= MAX_VALID_COLOR)
	{
		*(char *)(CONSOLE_MEM_BASE + 2 * (row * CONSOLE_WIDTH + col)) = ch; 
		*(char *)(CONSOLE_MEM_BASE + 2 * (row * CONSOLE_WIDTH + col) + 1) = color; 
	}
}

/** @brief Returns the character displayed at position (row, col).
 *  @param row Row of the character.
 *  @param col Column of the character.
 *  @return The character at (row, col).
 */
char get_char(int row, int col)
{
	if(row < CONSOLE_HEIGHT && col < CONSOLE_WIDTH)
	{
		return *(char *)(CONSOLE_MEM_BASE + 2 * (row * CONSOLE_WIDTH + col)); 
	}

	//FIXME Behavior for bad input is unspecified. 
	return 0;
}

/** @brief Changes the foreground and background color
 *         of future characters printed on the console.
 *
 *  If the color code is invalid, the function has no effect.
 *
 *  @param color The new color code.
 *  @return 0 on success or integer error code less than 0 if
 *          color code is invalid.
 */
int set_term_color(int color)
{
	if(color <= MAX_VALID_COLOR)
	{
		console_color = color;
		return 0;
	}
	return -1;
}

/** @brief Writes the current foreground and background
 *         color of characters printed on the console
 *         into the argument color.
 *  @param color The address to which the current color
 *         information will be written.
 *  @return Void.
 */
void get_term_color(int* color)
{
	//If color is invalid it's your own fault.
	*color = console_color;
}

/** @brief Sets the position of the cursor to the
 *         position (row, col).
 *
 *  Subsequent calls to putbytes should cause the console
 *  output to begin at the new position. If the cursor is
 *  currently hidden, a call to set_cursor() does not show
 *  the cursor.
 *
 *  @param row The new row for the cursor.
 *  @param col The new column for the cursor.
 *  @return 0 on success or integer error code less than 0 if
 *          cursor location is invalid.
 */
int set_cursor(int row, int col)
{
	if(row < CONSOLE_HEIGHT && col < CONSOLE_WIDTH)
	{
		console_row = row;
		console_col = col;

		if(!cursor_hidden)
			set_cursor_position(row, col);

		return 0;
	}
	return -1;
}

/** @brief Writes the current position of the cursor
 *         into the arguments row and col.
 *  @param row The address to which the current cursor
 *         row will be written.
 *  @param col The address to which the current cursor
 *         column will be written.
 *  @return Void.
 */
void get_cursor(int* row, int* col)
{
	//If row or col are invalid it's your own fault.
	*row = console_row;
	*col = console_col;
}

/** @brief Hides the cursor.
 *
 *  Subsequent calls to putbytes do not cause the
 *  cursor to show again.
 *
 *  @return Void.
 */
void hide_cursor()
{
	if(!cursor_hidden)
	{
		cursor_hidden = 1;
		set_cursor_position(CONSOLE_WIDTH, CONSOLE_HEIGHT);
	}
}

/** @brief Shows the cursor.
 *  
 *  If the cursor is already shown, the function has no effect.
 *
 *  @return Void.
 */
void show_cursor()
{
	if(cursor_hidden)
	{
		cursor_hidden = 0;
		set_cursor_position(console_row, console_col);
	}
}

/** @brief Clears the entire console.
 *
 * The cursor is reset to the first row and column
 *
 *  @return Void.
 */
void clear_console()
{
	char *out = (char*)CONSOLE_MEM_BASE;	
	
	while(out < CONSOLE_END)
	{
		*(out++) = ' ';
		*(out++) = console_color;
	}

	set_cursor(0,0);
}


