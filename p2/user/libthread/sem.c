/** 
* @file sem.c
* @brief Basic implementation of semaphores using one condition variable, and one mutex.
* @author Justin Scheiner
* @date 2010-09-27
*/

#include <thr_internals.h>
#include <sem.h>
#include <atomic.h>
#include <mutex.h>
#include <types.h>

static int sem_id = 0;

/** 
* @brief Initialize a semaphore. 
* 
* @param sem The semaphore to initialize.
* @param count The starting count for this semaphore.
* 
* @return 0 on success. 
* 		   -1 if the mutex is NULL.
* 			-2 if the semaphore is already initialized.
* 			-3 if we cannot initialize a new mutex.
* 			-4 if we cannot initialize a new condition variable.
*/
int sem_init(sem_t* sem, int count)
{
	int id;
	
	if(!sem) return -1;
	if(sem->initialized) return -2;
		
	sem->count = count;
	sem->initialized = TRUE;

	atomic_xadd(&id, &sem_id);
	sem->id = id;
	sem->waiting = 0;
	
	if(mutex_init(&sem->lock) != 0) return -3;
	if(cond_init(&sem->nonzero) != 0) return -4;

	return 0;
}

/** 
* @brief Deactivates a semaphore.
* 
* @param sem The semaphore to deactivate.
* 
* @return 0 on success.
* 			-1 if the semaphore is NULL.
* 		 	-2 if the semaphore wasn't initialized.
* 		 	-3 if we cannot destroy the mutex.
* 		 	-4 if we cannot destroy the condition.
*/
int sem_destroy( sem_t* sem )
{
	if(!sem)	return -1;
	if(!sem->initialized) return -2;
	
	sem->initialized = FALSE;
	
	if(mutex_destroy(&sem->lock) != 0) return -3;
	if(cond_destroy(&sem->nonzero) != 0) return -4;
	return 0;
}

/** 
* @brief Attempts to decrement the semaphores count. 
* 	
* 	If the semaphores count is zero, then blocks until it becomes nonzero.
* 
* @param sem The semaphore to decrement.
* 
* @return 0 on success.
*/
int sem_wait(sem_t* sem)
{
	mutex_lock(&sem->lock);
	if (sem->waiting > 0 || sem->count == 0) {
		sem->waiting++;
		cond_wait(&sem->nonzero, &sem->lock);
		sem->waiting--;
		assert (sem->count > 0);
	}
	atomic_add(&sem->count, -1);
	mutex_unlock(&sem->lock);
	return 0;
}

/** 
* @brief Increments the semaphores count, and signals the next waiting thread.
* 
* @param sem The semaphore to increment.
* 
* @return 0 on success.
*/
int sem_signal(sem_t* sem)
{
	atomic_add(&sem->count, 1);
	cond_signal(&sem->nonzero);
	return 0;
}



