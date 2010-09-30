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

/* readers/writers lock functions */
int rwlock_init( rwlock_t *rwlock )
{
	if(!rwlock) return -1;
	
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

int rwlock_destroy( rwlock_t *rwlock )
{
	mutex_destroy(&rwlock->clear_lock);
	cond_destroy(&rwlock->wait_write);	
	cond_destroy(&rwlock->wait_read);	
	return 0;
}


int rwlock_lock( rwlock_t *rwlock, int type )
{
	int writers, readers;
	if(!rwlock) return -1;
	
	switch(type)
	{
		case RWLOCK_READ: 
			
			// Wait for clearance to read from the active readers / writer
			mutex_lock(&rwlock->clear_lock);
			while(!rwlock->clear)
				cond_wait(&rwlock->wait_read, &rwlock->clear_lock);
			
			//Atomically increment the number of readers.
			readers = 1;
			atomic_xadd(&readers, &rwlock->readers);
			
			//Let other readers into the critical section if possible.
			rwlock->clear = TRUE;
			mutex_unlock(&rwlock->clear_lock);
			
			rwlock->mode = RWLOCK_READ;
			break;	
		
		case RWLOCK_WRITE: 
			
			// Wait for clearance to write from the active writer / readers.
			writers = 1;
			atomic_xadd(&writers, &rwlock->writers);
			
			mutex_lock(&rwlock->clear_lock);
			while(!rwlock->clear)
				cond_wait(&rwlock->wait_write, &rwlock->clear_lock);

			//Writers hold onto the clear lock until they exit.
			rwlock->mode = RWLOCK_WRITE;
			break;	
		
		default: return -2;
	}
	
	//We are free to read or write.
	return 0;
}

int rwlock_unlock( rwlock_t *rwlock )
{
	int writers, readers;
	
	if(!rwlock) return -1;

	switch(rwlock->mode)
	{
		case RWLOCK_WRITE: 

			//Decrement the number of writers.
			writers = -1;
			atomic_xadd(&writers, &rwlock->writers);
			
			// If the next writer doesn't beat us here, he doesn't get to run 
			// 	until the readers are done.
			if(rwlock->writers > 1)
			{
				cond_signal(&rwlock->wait_write);
			}
			else
			{
				cond_broadcast(&rwlock->wait_read);
			}
			//We still own the "clear" lock.
			rwlock->clear = TRUE;
			mutex_unlock(&rwlock->clear_lock);

			break;
		
		case RWLOCK_READ:
			
			//Decrement the number of active readers.
			readers = -1;
			atomic_xadd(&readers, &rwlock->readers);

			//If we were the last reader to decrement,
			if(readers == 1)
			{
				// Grab the clear lock.
				mutex_lock(&rwlock->clear_lock);

				//Let the first writer go if one is ready.
				cond_signal(&rwlock->wait_write);
				rwlock->clear = TRUE;
				mutex_unlock(&rwlock->clear_lock);
			}
			break;

		default: return -2;
	}
	return 0;
}


