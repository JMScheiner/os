/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <queue.h>
#include <mutex_type.h>
#include <types.h>

/* Condition variable error codes */

/** @brief Indicates a null condition variable was passed to a function. */
#define COND_NULL -11

/** @brief Indicates the condition variable was not in the proper state of
 * initialization. */
#define COND_INIT -12

/** Structures for condition variables **/
typedef struct cond_link
{
   /** @brief Tid of the thread that created this link. */
   int tid;

   /** @brief True iff the this link has been signaled. */
   boolean_t ready;

   /** @brief The next cond_link. */
   struct cond_link* next;

   /** @brief The previous cond_link. */
   struct cond_link* prev;
} cond_link_t;

/* A queue of links waiting to be signaled. */
DEFINE_QUEUE(cond_queue_t, cond_link_t*);

/** @brief A condition variable structure. */
typedef struct 
{
   /** @brief True iff the condition variable has been initialized. */
   boolean_t initialized;

   /** @brief A queue of waiting threads. */
   cond_queue_t q;

   /** @brief A mutual exclusion lock. */
   mutex_t qlock;

} cond_t;

#endif /* _COND_TYPE_H */

