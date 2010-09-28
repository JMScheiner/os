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

//#define MUTEX_DEBUG
#define THREAD_DEBUG

#ifdef MUTEX_DEBUG
	#include <stdio.h>
	#define mutex_debug_print(...) do { \
		lprintf(__VA_ARGS__); \
	} while(0)

#else

	#define mutex_debug_print(...) do { \
	} while(0)

#endif //MUTEX_DEBUG

#ifdef THREAD_DEBUG
	#include <stdio.h>
	#define thread_debug_print(...) do { \
		lprintf(__VA_ARGS__); \
	} while(0)

#else

	#define thread_debug_print(...) do { \
	} while(0)

#endif //THR_DEBUG

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
void clean_up_thread(tcb_t *tcb);
unsigned int prehash(char *addr);
int mutex_unlock_and_vanish(mutex_t* mp, char* int_stack);

tcb_t *thr_gettcb(boolean_t remove_tcb);

#endif /* THR_INTERNALS_H */


