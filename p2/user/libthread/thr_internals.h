/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <types.h>
#include <queue.h>
#include <mutex_type.h>

/** Structures for mutexes **/

/** 
* @brief Definition for a basic test and test-and-set lock.
*/
typedef struct 
{ 
	int lock;
	int tid;
}tts_lock_t;

/* 
 * A simple test and test-and-set lock implementation. 
 * 	Do not ensure bounded waiting.
 */
int tts_lock(tts_lock_t* lock);
int tts_try_lock(tts_lock_t* lock);
int tts_unlock(tts_lock_t* lock);
int tts_init(tts_lock_t* lock);
int tts_destroy(tts_lock_t* lock);

/** 
* @brief Useful for stack based mutex waiting lists.
*/
typedef struct _mnode { 
	tts_lock_t access;
	int tid;
	boolean_t cancel_deschedule;
	struct _mnode* next;
} mutex_node;


/** Structures for condition variables **/
typedef struct cond_link
{
	struct cond_link* next;
	struct cond_link* prev;
	int tid;
	boolean_t cancel_deschedule;
} cond_link_t;

DEFINE_QUEUE(cond_queue_t, cond_link_t*);

#endif /* THR_INTERNALS_H */


