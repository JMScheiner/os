/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include <cond.h>
#include <mutex.h>

/* Semaphore error codes */

/** @brief Indicates a null semaphore was passed to a function. */
#define SEM_NULL -21

/** @brief Indiactes the semaphore was not in the proper state of
 * initialization. */
#define SEM_INIT -22

/** @brief A semaphore structure */
typedef struct sem {

   /** @brief The number of open slots in the semaphore. */
   int open_slots;

   /** @brief A mutual exclusion lock for the semaphore. */
   mutex_t lock;

   /** @brief A condition variable used to signal an open slot in the semaphore. */
   cond_t nonzero;

   /** @brief The number of threads waiting to use the semaphore. */
   int waiting;

   /** @brief A unique id for the semaphore. */
   int id;

   /** @brief True iff the semaphore has been initialized. */
   boolean_t initialized;
} sem_t;

#endif /* _SEM_TYPE_H */

