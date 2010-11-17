/** 
* @file heap.h
* @brief A heap used for sleeping threads. 
*  The sleep heap keys on wake time. 
*
* @author Tim Wilson
* @author Justin Scheiner
* @date 2010-11-12
*/

#ifndef HEAP_FW3O8P69

#define HEAP_FW3O8P69

#include <kernel_types.h>

void heap_init(sleep_heap_t* heap);
int heap_check_size(sleep_heap_t *heap);
void heap_insert(sleep_heap_t* heap, tcb_t* key);
tcb_t* heap_pop(sleep_heap_t* heap);
tcb_t* heap_peek(sleep_heap_t* heap);
void heap_remove(sleep_heap_t* heap, tcb_t* key);

#endif /* end of include guard: HEAP_FW3O8P69 */


