/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <queue.h>
#include <mutex_type.h>
#include <types.h>

/** Structures for condition variables **/
typedef struct cond_link
{
	struct cond_link* next;
	struct cond_link* prev;
	int tid;
	boolean_t cancel_deschedule;
} cond_link_t;

DEFINE_QUEUE(cond_queue_t, cond_link_t*);

typedef struct 
{
	cond_queue_t q;
	mutex_t qlock;
} cond_t;


#endif /* _COND_TYPE_H */
