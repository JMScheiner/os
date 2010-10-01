/** 
* @file mutex.c
* @brief See function descriptions.
* @author Justin Scheiner
*/

#include <mutex.h>
#include <mutex_type.h>
#include <thread.h>
#include <thr_internals.h>
#include <syscall.h>
#include <atomic.h>
#include <assert.h>
#include <stddef.h>
#include <simics.h>
#include <stdio.h>
#include <simics.h>

static int mutex_id = 0;

/** 
* @brief Initialize a mutex for locking.
* 
* @param mp The mutex to initialize.
* 
* @return 0 on success, 
*         MUTEX_NULL if mp is NULL.
*         MUTEX_INIT if mp is already initialized.
*/
int mutex_init(mutex_t *mp)
{
	if(!mp) return MUTEX_NULL;
	if(mp->initialized == TRUE) return MUTEX_INIT;

	mp->id = atomic_add(&mutex_id, 1);
	mutex_debug_print("   .....Initialized mutex %d at address %p", mp->id, mp);
	mp->active_tid = NULL_TID;
	mp->ticket = 0;
	mp->now_serving = 0;
	mp->initialized = TRUE;
	
	return 0;
}

/** 
* @brief Destroy a mutex, e.g. deactivate it.
*        Blame the user for race conditions.
* 
* @param mp The mutex to destroy.
* 
* @return 0 on success, 
*         MUTEX_NULL if mp is NULL, 
*         MUTEX_INIT if mp wasn't active to begin with.
*         MUTEX_IN_USE if mp is locked.
*/
int mutex_destroy(mutex_t *mp)
{
	if(!mp) return MUTEX_NULL;
	if(mp->initialized == FALSE) return MUTEX_INIT;
	if(mp->ticket != mp->now_serving) return MUTEX_IN_USE;
	
	mp->initialized = FALSE;
	return 0;
}

/** 
* @brief Straightforward implementation of the bakery algorithm. 
*        Atomically take a ticket (enforces bounded waiting), 
*        then wait for "now_serving" to match your ticket. 
*
* @param mp The mutex to lock.
* 
* @return 0 on success, 
* 			  MUTEX_NULL if mp is NULL, 
* 			  MUTEX_INIT if mp isn't initialized.
*/
int mutex_lock( mutex_t *mp )
{
	if(!mp) return MUTEX_NULL;
	if(mp->initialized == FALSE) return MUTEX_INIT;
	
	int tid = thr_getid();
	int ticket = atomic_add(&mp->ticket, 1);

	while(ticket != mp->now_serving)
		thr_yield(mp->active_tid);
	
	mp->active_tid = tid;
	return 0;
}

/** 
* @brief Increment now_serving to unlock the mutex, and leave.
* 
* @param mp The mutex to unlock
* 
* @return 0 on success
*         MUTEX_NULL if mp is NULL.
* 			  MUTEX_INIT if mp is not initialized.
*/
int mutex_unlock( mutex_t *mp )
{
	if(!mp) return -1;
	if(mp->initialized == FALSE) return -2;
	
	/* Make sure the scheduler yields to people other than me. */
	mp->active_tid = NULL_TID;
	mp->now_serving++;
	return 0;
}

