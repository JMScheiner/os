/** @file hashtable.h
 *
 * @author Tim Wilson (tjwilson)
 */

#ifndef HASHTABLE_H_DSYG34FFF
#define HASHTABLE_H_DSYG34FFF

#include <kernel_types.h>

unsigned int default_hash(int tid);
void hashtable_init(hashtable_t *hashtable, unsigned int (*hash)(int));
void hashtable_put(hashtable_t *hashtable, int tid, tcb_t *tcb);
tcb_t *hashtable_get(hashtable_t *hashtable, int tid);
tcb_t *hashtable_remove(hashtable_t *hashtable, int tid);

#endif

