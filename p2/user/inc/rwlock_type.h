/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <types.h>
#include <cond.h>
#include <mutex.h>

/** @brief Indicates a null rwlock was passed to a function. */
#define RWLOCK_NULL -13

/** @brief Indicates the reader writer lock was not initialized. */
#define RWLOCK_INIT -14

/** @brief Indicates an attempt to lock with an invalid type. */
#define RWLOCK_INVALID_TYPE -15

typedef struct rwlock 
{
	/* @brief The active mode of the reader writer lock. RWLOCK_READ or RWLOCK_WRITE*/
	int mode;

	/* @brief The number of waiting writers */
	int writers;

	/* @brief The number of active readers in the critical section*/
	int readers;
	
	/* @brief A condition indicating the number of waiting writers is 0
	 * 	...otherwise, we are free to read. */ 
	cond_t wait_read;
	
	/* @brief The condition that there are no active readers. */ 
	cond_t wait_write; 
	
	/* @brief Protects access to the number of readers and writers. */
	mutex_t rw_count_lock;
	
	/* @brief A flag indicating that this struct is initialized. */
	boolean_t initialized;

} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
