
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


