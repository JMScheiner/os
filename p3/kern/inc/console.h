#ifndef CONSOLE_IRQS5C5P
#define CONSOLE_IRQS5C5P

int 	putbyte(char ch);
void 	putbytes(const char* s, int len);
void 	draw_char(int row, int col, int ch, int color);
char 	get_char(int row, int col);
int 	set_term_color(int color);
void	get_term_color(int* color);
int	set_cursor(int row, int col);
void 	get_cursor(int* row, int* col);
void 	hide_cursor(void);
void 	show_cursor(void);
void 	clear_console(void);
void 	scroll_console(void);

#endif /* end of include guard: CONSOLE_IRQS5C5P */

