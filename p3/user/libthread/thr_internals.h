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
//#define THREAD_DEBUG

#ifdef MUTEX_DEBUG
   #include <stdio.h>
   /** @brief Debugging macro */
   #define mutex_debug_print(...) do { \
      lprintf(__VA_ARGS__); \
   } while(0)

#else

   #define mutex_debug_print(...) do { \
   } while(0)

#endif //MUTEX_DEBUG

#ifdef THREAD_DEBUG
   #include <stdio.h>
   /** @brief Debugging macro */
   #define thread_debug_print(...) do { \
      lprintf(__VA_ARGS__); \
   } while(0)

#else

   #define thread_debug_print(...) do { \
   } while(0)

#endif //THR_DEBUG

/** @brief The tid of no thread. This should be yielded to if we need
 * to yield, but don't know who to yield to. */
#define NULL_TID -1

/* Thread control block */

/** @brief Thread control block. Stores information about a thread. */
typedef struct tcb {

   /** @brief The tid of this thread. */
   int tid;

   /** @brief Optional status returned by this thread in thr_exit. */
   void *statusp;

   /** @brief A lock for this block. */
   mutex_t lock;

   /** @brief A signal to indicate that this thread has been initialized. */
   cond_t init_signal;

   /** @brief A signal to indicate that this thread has exited. */
   cond_t exit_signal;
   
   /** @brief TRUE iff this thread has/is exiting. The thread's status must be
    * set before this is set to TRUE. */
   boolean_t exited;

   /** @brief TRUE iff all fields of the tcb have been initialized. */
   boolean_t initialized;
} tcb_t;

void thr_child_init(void *(*func)(void*), void* arg, tcb_t* tcb);
void wait_for_child(tcb_t *tcb);
tcb_t **thr_gettcb();
void clean_up_thread(int tid, char *old_stack);

/** @brief Unlock the given mutex, jump to the given stack, and vanish
 *
 * @param mp The mutex to unlock. This mutex was protecting our current stack.
 * @param int_stack The stack to jump to immediately before calling vanish.
 */
int mutex_unlock_and_vanish(mutex_t* mp, char* int_stack);

#endif /* THR_INTERNALS_H */

