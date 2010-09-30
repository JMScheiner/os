
#ifndef LOCK_FREE_LIST_H
#define LOCK_FREE_LIST_H

#include <thr_internals.h>

/** @brief Opaque linked list abstract data type */
typedef struct linked_list *list_t;

list_t list_new();
tcb_t *list_lookup(list_t list, unsigned int key);
tcb_t *list_delete(list_t list, unsigned int key);
void list_insert(list_t list, unsigned int key, tcb_t *val);

#endif

