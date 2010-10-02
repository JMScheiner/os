/** 
* @file cond.c
* @brief Condition variable library.
* 	Maintains a locked queue of waiting threads.
*
* @author Justin Scheiner 
* @date 2010-09-21
*/

#include <cond.h>
#include <cond_type.h>
#include <mutex.h>
#include <thread.h>
#include <syscall.h>
#include <thr_internals.h>
#include <simics.h>

/** 
* @brief Initializes the internal queue and 
* 	it's associated mutex. 
* 
* @param cv The condition variable to initialize.
* 
* @return 0 on success
*         COND_NULL if cv is NULL.
*         COND_INIT if cv is already initialized.
*/
int cond_init( cond_t* cv)
{
	if(!cv) return COND_NULL;
	if(cv->initialized) return COND_INIT;

	cv->initialized = TRUE;
	
	mutex_init(&cv->qlock);
	STATIC_INIT_QUEUE(cv->q);
	return 0;
}

/** 
* @brief Destroys the given condition variable.
* 
* @param cv The condition variable to destroy.
* 
* @return 0 on success.
*         COND_NULL if cv is NULL.
*         COND_INIT if cv isn't already active.
*/
int cond_destroy( cond_t* cv)
{
	if(!cv) return COND_NULL;
	if(!cv->initialized) return COND_INIT;
	
	cv->initialized = FALSE;
	mutex_destroy(&cv->qlock);
	
	return 0;
}

/** 
* @brief Blocks until a cond_signal or cond_broadcast awakens the thread.
* 	1. Adds this thread to the run queue.
* 	2. Releases the associated lock.
* 	3. Safely deschedules.
* 	4. Upon waking up, reaquires the associated lock.
* 	 
* @param cv The condition variable to wait on.
* @param mp The associated mutex.
* 
* @return 0 on success.
*         COND_NULL if cv is NULL.
*         MUTEX_NULL if mp is NULL.
*         COND_INIT if cv is not initialized.
*         MUTEX_INIT if mp is not initialized.
*         < 0 if mp does not lock or unlock successfully.
*/
int cond_wait( cond_t* cv, mutex_t* mp )
{
	int ret;

	if(!cv) return COND_NULL;
	if(!mp) return MUTEX_NULL;
	
	if(!cv->initialized) return COND_INIT;
	if(!mp->initialized) return MUTEX_INIT;
		
	cond_link_t link;
	link.tid = thr_getid();
	link.ready = FALSE;
	
	/* Add ourself to the end of the waiting list. */
	mutex_lock(&cv->qlock);
	ENQUEUE_LAST(cv->q, &link);
	mutex_unlock(&cv->qlock);
	
	if((ret = mutex_unlock(mp)) != 0)
	{
		return ret;
	}

	/* Deschedule ourselves until we are woken up. */
	while (link.ready == FALSE) {
		deschedule((int*)&link.ready);
	}

	if ((ret = mutex_lock(mp)) != 0)
	{
		return ret;
	}

	return 0;
}

/** 
* @brief Signals one thread waiting on the condition to continue.
* 	Finds the first person in the run queue, and lets him run.
* 
* @param cv The condition variable to signal.
* 
* @return 0 on success. 
*         COND_NULL if cv is NULL.
*         COND_INIT if cv is not initialized.
*/
int cond_signal( cond_t* cv )
{
	if(!cv) return COND_NULL;
	if(!cv->initialized) return COND_INIT;
	
	cond_link_t* link;
	
	/* Get the first waiting thread and wake them up. */
	mutex_lock(&cv->qlock);
	DEQUEUE_FIRST(cv->q, link);
	mutex_unlock(&cv->qlock);
	
	if(link)
	{
		link->ready = TRUE;
		make_runnable(link->tid);
	}
	
	return 0;
}

/** 
* @brief Signals all threads waiting on the condition to continue.
* 	Locks and dequeues all of the the threads on the run queue.
* 
* @param cv The condition variable to broadcast for.
* 
* @return 0 on success.
*         COND_NULL if cv is NULL
*         COND_INIT if cv is not initialized
*/
int cond_broadcast( cond_t* cv)
{
	if(!cv) return COND_NULL;
	if(!cv->initialized) return COND_INIT;

	cond_link_t* link;

	mutex_lock(&cv->qlock);

	/* Iterate through all waiting threads and wake them up. */
	FOREACH(cv->q, link)
	{
		link->ready = TRUE;
		make_runnable(link->tid);
	}

	EMPTY_QUEUE(cv->q);
	mutex_unlock(&cv->qlock);
	
	return 0;
}


