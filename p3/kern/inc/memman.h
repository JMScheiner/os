/** 
* @file memman.h
* @brief Defines memory management handlers 
*  new_pages and remove_pages
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-11-12
*/
#ifndef MEMMAN_ZSQTJ8CD

#define MEMMAN_ZSQTJ8CD

#include <kernel_types.h>
#include <reg.h>

#define NEW_PAGES_INVALID_ARGS -1

mutex_t* new_pages_lock(void);

void memman_init(void);
void new_pages_handler(volatile regstate_t reg);
void remove_pages_handler(volatile regstate_t reg);

#endif /* end of include guard: MEMMAN_ZSQTJ8CD */



