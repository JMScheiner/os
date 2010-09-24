/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <cond_type.h>
#include <types.h>
#include <queue.h>
#include <mutex_type.h>

#define THR_DEBUG

#ifdef THR_DEBUG
	#include <stdio.h>
	#define thr_debug_print(...) do { \
		printf(__VA_ARGS__); \
	} while(0)

#else

	#define thr_debug_print(...) do { \
	} while(0)

#endif //THR_DEBUG

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

/* Thread control block */

/** @brief Thread control block. Stores information about a thread. */
typedef struct tcb {

	/** @brief The stack this thread is executing on. This will be NULL for the
	 * main thread. */
	char *stack;

	/** @brief The tid of this thread. */
	int tid;

	/** @brief Optional status returned by this thread in thr_exit. */
	void *statusp;

	/** @brief A lock for this block. */
	mutex_t lock;

	/** @brief A signal to indicate that this thread has exited. */
	cond_t signal;

	/** @brief TRUE iff this thread has/is exiting. The thread's status must be
	 * set before this is set to TRUE. */
	boolean_t exited;

	/** @brief TRUE iff all fields of the tcb have been initialized. */
	boolean_t initialized;
} tcb_t;

void thr_child_init(void *(*func)(void*), void* arg, tcb_t* tcb);
void wait_for_child(tcb_t *tcb);
int mutex_unlock_and_vanish(mutex_t* mp);

tcb_t *thr_gettcb(boolean_t remove_tcb);

#endif /* THR_INTERNALS_H */


