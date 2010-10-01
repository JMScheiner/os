/** 
* @file cond.c
* @brief Condition variable library.
* 	Maintains a locked queue of waiting threads.
*
* @author Justin Scheiner 
* @date 2010-09-21
*/

#include <cond.h>
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
*        -1 if cv is NULL.
* 		   -2 if it is already initialized.
*/
int cond_init( cond_t* cv)
{
	if(!cv) return -1;
	if(cv->initialized) return -2;

	cv->initialized = TRUE;
	
	mutex_init(&cv->qlock);
	STATIC_INIT_QUEUE(cv->q);
	return 0;
}

/** 
* @brief Destroys the associated mutex.
* 
* @param cv The condition variable to destroy.
* 
* @return 0 on success.
*        -1 if cv is NULL.
* 		   -2 if cv isn't already active.
*/
int cond_destroy( cond_t* cv)
{
	if(!cv) return -1;
	if(!cv->initialized) return -2;
	
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
*        -1 if cv is NULL.
*        -2 if mp is NULL.
*        -3 if cv is not initialized.
*        -4 if mp is not initialized.
*/
int cond_wait( cond_t* cv, mutex_t* mp )
{
	if(!cv) return -1;
	if(!mp) return -2;
	
	if(!cv->initialized) return -3;
	if(!mp->initialized) return -4;
		
	cond_link_t link;
	link.tid = thr_getid();
	link.cancel_deschedule = FALSE;
	
	mutex_lock(&cv->qlock);
	ENQUEUE_LAST(cv->q, &link);
	mutex_unlock(&cv->qlock);
	
	if(mutex_unlock(mp) != 0) 
	{
		MAGIC_BREAK;
	}
	
	deschedule((int*)&link.cancel_deschedule);
	mutex_lock(mp);

	return 0;
}

/** 
* @brief Signals one thread waiting on the condition to continue.
* 	Finds the first person in the run queue, and lets him run.
* 
* @param cv The condition variable to signal.
* 
* @return 0 on success. 
*        -1 if cv is NULL.
*        -2 if cv is not initialized.
*/
int cond_signal( cond_t* cv )
{
	if(!cv) return -1;
	if(!cv->initialized) return -2;
	
	cond_link_t* link;
	
	mutex_lock(&cv->qlock);
	DEQUEUE_FIRST(cv->q, link);
	mutex_unlock(&cv->qlock);
	
	if(link)
	{
		link->cancel_deschedule = TRUE;
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
* 	      -1 if cv is NULL
* 	      -2 if cv is not initialized
*/
int cond_broadcast( cond_t* cv)
{
	if(!cv) return -1;
	if(!cv->initialized) return -2;

	cond_link_t* link;

	mutex_lock(&cv->qlock);
	FOREACH(cv->q, link)
	{
		link->cancel_deschedule = TRUE;
		make_runnable(link->tid);
	}

	EMPTY_QUEUE(cv->q);
	mutex_unlock(&cv->qlock);
	
	return 0;
}


