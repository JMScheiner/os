/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <queue.h>

/** 
* @brief Definition for a basic test and test-and-set lock.
*/
typedef int tts_lock_t;

/* A simple test and test-and-set lock implementation. 
 * 	These will be useful later when doing the bounded
 * 	waiting version of mutexes, but are also a fair stand-in
 * 	until that code is written.
 */
int tts_lock(tts_lock_t* lock);
int tts_try_lock(tts_lock_t* lock);
int tts_unlock(tts_lock_t* lock);
int tts_init(tts_lock_t* lock);
int tts_destroy(tts_lock_t* lock);

typedef struct _mnode { 
	int tid;
	struct _mnode* next;
} mutex_node;


/** 
* @brief Thread control block - should be created during
* 	thr_create, and put either at the top of the threads
* 	stack area, or in the heap independently.
*/
typedef struct _TCB_
{
	int tid;	
	
	//Maybe some other useful things like a stack pointer.
	
	//Useful things for mutexes: 
	int dont_deschedule;
	struct _TCB_* next;
	struct _TCB_* prev;
} tcb_t;

/** 
* @brief Should return the TCB for this thread.
* 
* @return The TCB for the running thread.
*/
tcb_t* get_tcb();

//Queue definitions:
DEFINE_QUEUE(tcb_queue, tcb_t*);

#endif /* THR_INTERNALS_H */


