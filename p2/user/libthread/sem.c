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

/** @brief An id counter. */
static int sem_id = 0;

/** 
* @brief Initialize a semaphore. 
* 
* @param sem The semaphore to initialize.
* @param count The starting count for this semaphore.
* 
* @return 0 on success. 
*         SEM_NULL if the semaphore is NULL.
*         SEM_INIT if the semaphore is already initialized.
*         MUTEX_INIT if we cannot initialize a new mutex.
*         COND_INIT if we cannot initialize a new condition variable.
*/
int sem_init(sem_t* sem, int count)
{
	if(!sem) return SEM_NULL;
	if(sem->initialized) return SEM_INIT;
		
	sem->open_slots = count;
	sem->initialized = TRUE;

	sem->id = atomic_add(&sem_id, 1);
	sem->waiting = 0;
	
	if(mutex_init(&sem->lock) != 0) return MUTEX_INIT;
	if(cond_init(&sem->nonzero) != 0) return COND_INIT;

	return 0;
}

/** 
* @brief Deactivates a semaphore.
* 
* @param sem The semaphore to deactivate.
* 
* @return 0 on success.
*         SEM_NULL if the semaphore is NULL.
*         SEM_INIT if the semaphore wasn't initialized.
*         MUTEX_INIT if we cannot destroy the mutex.
*         COND_INIT if we cannot destroy the condition.
*/
int sem_destroy( sem_t* sem )
{
	if(!sem) return SEM_NULL;
	if(!sem->initialized) return SEM_INIT;
	sem->initialized = FALSE;
	if(mutex_destroy(&sem->lock) != 0) return MUTEX_INIT;
	if(cond_destroy(&sem->nonzero) != 0) return COND_INIT;
	return 0;
}

/** 
* @brief Attempts to decrement the semaphores count. 
* 	
* If the semaphores count is zero, then blocks until it becomes nonzero.
* 
* @param sem The semaphore to decrement.
* 
* @return 0 on success.
* 				< 0 on failure. Failure may occur if the semaphore's mutex or
* 				condition variable fails.
*/
int sem_wait(sem_t* sem)
{
	int ret;
	if ((ret = mutex_lock(&sem->lock)) != 0)
		return ret;
	if (sem->waiting > 0 || sem->open_slots == 0) {
		sem->waiting++;
		if ((ret = cond_wait(&sem->nonzero, &sem->lock)) != 0)
			return ret;
		sem->waiting--;
	}
	atomic_add(&sem->open_slots, -1);
	return mutex_unlock(&sem->lock);
}

/** 
* @brief Increments the semaphores count, and signals the next waiting thread.
* 
* @param sem The semaphore to increment.
* 
* @return 0 on success.
*         < 0 on failure. Failure may occur if the semaphore's condition
*         variable fails.
*/
int sem_signal(sem_t* sem)
{
	atomic_add(&sem->open_slots, 1);
	return cond_signal(&sem->nonzero);
}



