
#include <console.h>
#include <video_defines.h>
#include <asm.h>
#include <debug.h>
#include <keyboard.h>
#include <mm.h>

#define CONSOLE_END ((char*)(CONSOLE_MEM_BASE + 2 * CONSOLE_WIDTH * CONSOLE_HEIGHT))
#define MAX_VALID_COLOR 0x8f

#define READLINE_INVALID_ARGS -1
#define READLINE_INVALID_LENGTH -2
#define READLINE_INVALID_BUFFER -3

//Here's the console state: 
static int console_color = FGND_WHITE | BGND_BLACK;

//Invariant: Always on the next position to write.
static int console_row = 0;
static int console_col = 0;
static int cursor_hidden = 1;


/************** Syscall wrappers. **************/
void getchar_handler(volatile regstate_t reg)
{
	lprintf("Ignoring getchar");
	MAGIC_BREAK;
   //TODO
}

void readline_handler(volatile regstate_t reg)
{
	char *arg_addr = (char *)SYSCALL_ARG(reg);
	if (!mm_validate(arg_addr) || !mm_validate(arg_addr + sizeof(int))) {
		RETURN(READLINE_INVALID_ARGS);
	}

	int len = *(int *)arg_addr;
	if (len < 0 || len > KEY_BUF_SIZE) {
		RETURN(READLINE_INVALID_LENGTH);
	}

	char *buf = *(char **)(arg_addr + sizeof(int));
	if (!mm_validate_write(buf, len)) {
		RETURN(READLINE_INVALID_BUFFER);
	}

	debug_print_readline("%0x%x: reading up to %d chars to %p\n", 
			get_tcb()->tid, len, buf);

	int read = readline(buf, len);
	RETURN(read);
}

void print_handler(volatile regstate_t reg)
{
	lprintf("Ignoring print");
	MAGIC_BREAK;
   //TODO
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


