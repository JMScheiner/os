/** 
* @file malloc_wrappers.h
*
* @brief Malloc wrappers. Keeps track of 
*  memory smalloc'd and freed to track down memory leaks.
*
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-11-12
*/
#ifndef MALLOC_WRAPPER_H_FD234GHJ
#define MALLOC_WRAPPER_H_FD234GHJ

void alloc_init();
void *scalloc(size_t nmemb, size_t size);
void *srealloc(void* buf, size_t current_size, size_t new_size);

#endif

