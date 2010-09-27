/** 
* @file mutex.c
* @brief See function descriptions.
* @author Justin Scheiner
*/

#include <mutex.h>
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
* 			-1 if the mutex is invalid.
* 			-2 if the mutex is already initialized.
*/
int mutex_init(mutex_t *mp)
{
	int id = 1;
	
	if(!mp) return -1;
	if(mp->initialized == TRUE) return -2;
	
	atomic_xadd(&id, &mutex_id);
	mutex_debug_print("   .....Initialized %d", id);
	mp->id = id;
	mp->ticket = 0;
	mp->now_serving = 0;
	mp->owner_prehash = 0;
	mp->initialized = TRUE;
	
	return 0;
}

/** 
* @brief Destroy a mutex, e.g. deactivate it.
* 	Blame the user for race conditions.
* 
* @param mp The mutex to destroy.
* 
* @return 0 on success, 
* 	-1 if mp is NULL, 
* 	-2 if the lock is in use.
* 	-3 if the lock wasn't active to begin with.
*/
int mutex_destroy(mutex_t *mp)
{
	if(!mp) return -1;
	if(mp->owner_prehash) return -2;
	if(mp->initialized == FALSE) return -3;
	
	mp->initialized = FALSE;
	return 0;
}

/** 
* @brief Straightforward implementation of the bakery algorithm. 
* 	Atomically take a ticket (enforces bounded waiting), 
* 	 then wait for "now_serving" to match your ticket. 
*
* 	Releasing the lock is equivalent to incrementing now_serving.
*
*  Relies on a fair scheduler, since we don't know the tid (to avoid 
*   making an expensive system call).
* 
* @param mp The mutex to lock.
* 
* @return 0 on success, 
* 			-1 if the lock isn't valid, 
* 			-2 if the lock is already owned by the calling thread.
* 			-3 if the lock isn't initialized.
*/
int mutex_lock( mutex_t *mp )
{
	int ticket = 1;
	int my_prehash;
	
	if(!mp) return -1;
	if(mp->initialized == FALSE) return -3;
	
	//Check if we already own the lock.
	my_prehash = prehash((char*)&ticket);
	if(mp->owner_prehash == my_prehash || mp->owner_prehash == my_prehash - 1 
												  || mp->owner_prehash == my_prehash + 1)
		return -2;

	atomic_xadd(&ticket, &mp->ticket);
	
	mutex_debug_print("[%d] Waiting for lock %d, ticket = %d, now_serving = %d", 
		my_prehash, mp->id, ticket, mp->now_serving);

	while(ticket != mp->now_serving)
		yield(-1);

	/* We have acquired the lock. */
	mp->owner_prehash = my_prehash;
	mutex_debug_print("[%d] Acquires lock %d, ticket = %d, now_serving = %d", 
		my_prehash, mp->id, ticket, mp->now_serving);
	
	return 0;
}

/** 
* @brief Increment now_serving, and leave.
* 
* @param mp 
* 
* @return -1 if mp is invalid.
* 			 -3 if mp is not initialized.
* 			 -2 if the calling thread does not own the lock.
*/
int mutex_unlock( mutex_t *mp )
{
	int my_prehash;
	
	if(!mp) return -1;
	if(mp->initialized == FALSE) return -3;
	
	//Check if we actually own the lock.
	my_prehash = prehash((char*)&my_prehash);
	if(mp->owner_prehash != my_prehash && mp->owner_prehash != my_prehash + 1
												  && mp->owner_prehash != my_prehash - 1)
	{
		printf("Expected prehash %d, got prehash %d\n", mp->owner_prehash, my_prehash);
		return -2;
	}

	mutex_debug_print("[%d] Releases lock %d - ticket = %d, now_serving = %d++", 
		my_prehash, mp->id, mp->ticket, mp->now_serving);
	mp->owner_prehash = 0;
	mp->now_serving++;
	return 0;
}

