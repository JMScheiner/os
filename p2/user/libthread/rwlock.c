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
*        -1 if rwlock is NULL.
*        -2 if rwlock is already initialized.
*/
int rwlock_init( rwlock_t *rwlock )
{
	if(!rwlock) return -1;
	if(rwlock->initialized) return -2;
	
	// This value doesn't matter, since clear will be true.
	rwlock->mode = RWLOCK_READ;
	
	// This passes people through when no one is waiting. 	
	rwlock->clear = TRUE;

	rwlock->writers = 0;
	rwlock->readers = 0;
	rwlock->initialized = TRUE;
	mutex_init(&rwlock->clear_lock);
	cond_init(&rwlock->wait_write);	
	cond_init(&rwlock->wait_read);	
	
	return 0;
}

/** 
* @brief Deactivates a reader-writer lock.
* 
* @param rwlock The reader-writer lock to deactivate.
* 
* @return 0 on success.
*        -1 if rwlock is NULL.
*        -2 if rwlock isn't initialized.
*/
int rwlock_destroy( rwlock_t *rwlock )
{
	if(!rwlock) return -1;
	if(!rwlock->initialized) return -2;

	mutex_destroy(&rwlock->clear_lock);
	cond_destroy(&rwlock->wait_write);	
	cond_destroy(&rwlock->wait_read);	
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
*        -1 if the lock is NULL.
*        -2 if the lock is not initialized.
*/
int rwlock_lock( rwlock_t *rwlock, int type )
{
	if(!rwlock) return -1;
	if(!rwlock->initialized) return -2;
	
	switch(type)
	{
		case RWLOCK_READ: 
			
			// Wait for clearance to read from the active readers / writer
			mutex_lock(&rwlock->clear_lock);
			while(!rwlock->clear || rwlock->writers > 0)
				cond_wait(&rwlock->wait_read, &rwlock->clear_lock);
			
			//Atomically increment the number of readers.
			atomic_add(&rwlock->readers, 1);
			
			//Let other readers into the critical section if possible.
			rwlock->clear = TRUE;
			mutex_unlock(&rwlock->clear_lock);
			
			rwlock->mode = RWLOCK_READ;
			break;	
		
		case RWLOCK_WRITE: 
			
			// Wait for clearance to write from the active writer / readers.
			atomic_add(&rwlock->writers, 1);
			
			mutex_lock(&rwlock->clear_lock);
			while(!rwlock->clear || rwlock->readers > 0)
				cond_wait(&rwlock->wait_write, &rwlock->clear_lock);

			//Writers hold onto the clear lock until they exit.
			rwlock->mode = RWLOCK_WRITE;
			break;	
		
		default: return -2;
	}
	
	//We are free to read or write.
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
* 			-1 if the lock is NULL.
* 			-2 if the lock isn't initialized.
*/
int rwlock_unlock( rwlock_t *rwlock )
{
	int readers;
	if(!rwlock) return -1;
	if(!rwlock->initialized) return -2;

	switch(rwlock->mode)
	{
		case RWLOCK_WRITE: 

			//Decrement the number of writers.
			atomic_add(&rwlock->writers, -1);
			
			// If the next writer doesn't beat us here, he doesn't get to run 
			// 	until the readers are done.
			rwlock->clear = TRUE;
			if(rwlock->writers > 1)
			{
				cond_signal(&rwlock->wait_write);
			}
			else
			{
				cond_broadcast(&rwlock->wait_read);
			}
			//We still own the "clear" lock.
			mutex_unlock(&rwlock->clear_lock);

			break;
		
		case RWLOCK_READ:
			
			//Decrement the number of active readers.
			readers = atomic_add(&rwlock->readers, -1);

			//If we were the last reader to decrement,
			if(readers == 1)
			{
				// Grab the clear lock.
				mutex_lock(&rwlock->clear_lock);

				//Let the first writer go if one is ready.
				rwlock->clear = TRUE;
				cond_signal(&rwlock->wait_write);
				mutex_unlock(&rwlock->clear_lock);
			} 

			break;

		default: return -2;
	}
	return 0;
}


