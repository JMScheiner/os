/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <queue.h>
#include <mutex_type.h>
#include <types.h>

/* Condition variable error codes */
#define COND_NULL -10
#define COND_INIT -11
#define COND_MUTEX_UNLOCK -12
#define COND_MUTEX_LOCK -13

/** Structures for condition variables **/
typedef struct cond_link
{
	int tid;
	struct cond_link* next;
	struct cond_link* prev;
	boolean_t ready;
} cond_link_t;

DEFINE_QUEUE(cond_queue_t, cond_link_t*);

typedef struct 
{
	boolean_t initialized;
	cond_queue_t q;
	mutex_t qlock;
} cond_t;

#endif /* _COND_TYPE_H */
