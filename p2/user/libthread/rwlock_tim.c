/** 
* @file rwlock2.c
* @brief Rules for rwlocks: 
* 	1. The last reader is obligated to signal the first writer.
* 	2. Only the last writer can let the readers read.
* 		- Subsequent writers have to be on the list before they can block readers.
*/

#include <rwlock.h>
#include <atomic.h>
#include <thr_internals.h>
#include <simics.h>
#include <thread.h>

/** 
* @brief Initializes a reader-writer lock.
* 	Specifically initializes the internal condition variables 
* 	associated with readers and writers, as well as a mutex 
* 	for those condition variables.
* 
* @param rwlock The reader-writer lock to initialize.
* 
* @return 0 on success. 
*        RWLOCK_NULL if rwlock is NULL.
*        RWLOCK_INIT if rwlock is already initialized.
*/
int rwlock_init( rwlock_t *rwlock )
{
	if(!rwlock) return RWLOCK_NULL;
	if(rwlock->initialized) return RWLOCK_INIT;
	
	// This value doesn't matter, since clear will be true.
	rwlock->mode = RWLOCK_READ;
	
	rwlock->readers = 0;
	rwlock->initialized = TRUE;
	
	return 0;
}

/** 
* @brief Deactivates a reader-writer lock.
* 
* @param rwlock The reader-writer lock to deactivate.
* 
* @return 0 on success.
*        RWLOCK_NULL if rwlock is NULL.
*        RWLOCK_INIT if rwlock isn't initialized.
*/
int rwlock_destroy( rwlock_t *rwlock )
{
	if(!rwlock) return RWLOCK_NULL;
	if(!rwlock->initialized) return RWLOCK_INIT;

	return 0;
}


/** 
* @brief Attempts to lock a reader writer lock for reading or writing.
*
* Writers block readers, unless a read is already happening. 
* Both types wait on a condition variable that can be signalled when appropriate.
* There is also an atomic "reader" count that will signal writers when it drops to zero.
* 
* @param rwlock The reader writer lock.
* @param type RWLOCK_READ for reading, RWLOCK_WRITE for writing.
* 
* @return 0 on success.
*        RWLOCK_NULL if the lock is NULL.
*        RWLOCK_INIT if the lock is not initialized.
*/
int rwlock_lock( rwlock_t *rwlock, int type )
{
	if(!rwlock) return RWLOCK_NULL;
	if(rwlock->initialized == FALSE) return RWLOCK_INIT;
	
	int ticket = atomic_add(&rwlock->ticket, 1);

	while(ticket != rwlock->now_serving)
		thr_yield(NULL_TID);
	
	rwlock->mode = type;
	switch (type) {
		case RWLOCK_READ:
			rwlock->mode = type;
			atomic_add(&rwlock->readers, 1);
			rwlock->now_serving++;
			break;
		case RWLOCK_WRITE:
			while (rwlock->readers > 0)
				thr_yield(NULL_TID);
			rwlock->mode = type;
			break;
		default:
			return RWLOCK_INVALID_TYPE;
	}

	return 0;
}

/** 
* @brief Unlocks a reader writer lock.
*  If we are in read mode, 
*   Decrement the number of readers and signal a writer if there is one.
*
*  If we are in write mode, 
*   Signal a waiting writer if there is one, otherwise broadcast to readers
*   that the lock is available.
* 
* @param rwlock The lock to release.
* 
* @return 0 on success.
* 			RWLOCK_NULL if the lock is NULL.
* 			RWLOCK_INIT if the lock isn't initialized.
*/
int rwlock_unlock( rwlock_t *rwlock )
{
	switch (rwlock->mode) {
		case RWLOCK_READ:
			atomic_add(&rwlock->readers, -1);
			break;
		case RWLOCK_WRITE:
			rwlock->now_serving++;
		default:
			return RWLOCK_INVALID_TYPE;
	}
	return 0;
}

