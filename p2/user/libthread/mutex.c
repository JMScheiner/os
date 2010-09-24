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
* @return 0 on success, -1 on failure.
*/
int mutex_init(mutex_t *mp)
{
	mutex_id++;
	mp->ticket = 0;
	mp->now_serving = 0;
	mp->active_tid = -1;
	mp->initialized = TRUE;
	return 0;
}

/** 
* @brief Destroy a mutex, e.g. deactivate it.
* 
* @param mp The mutex to destroy.
* 
* @return 0 on success, 
* 	-1 if mp is NULL, 
* 	-2 if the lock is in use.
*/
int mutex_destroy(mutex_t *mp)
{
	mp->initialized = FALSE;
	return 0;
}

/** 
* @brief Try locking the mutex.
* 	Not implemented yet.
* 
* @param mp The mutex to try and lock.
* 
* @return 0 on success
* 	-1 on failure.
*/
int mutex_try_lock(mutex_t *mp)
{
	assert(mp);
	//TODO
	return 0;
}


int mutex_lock( mutex_t *mp )
{
	//int tid = gettid();
	int ticket = 1;

	atomic_xadd(&ticket, &mp->ticket);
	
	while(ticket != mp->now_serving)
	{
		//yield(mp->active_tid);
		yield(-1);
	}
	//BANG race condition. We could yield to the wrong person.

	//We have the mutex.
	//mp->active_tid = tid;
	return 0;
}

int mutex_unlock( mutex_t *mp )
{
	int now_serving = 1;
	atomic_xadd(&now_serving, &mp->now_serving);
	return 0;
}

/** 
* @brief Attempts to lock the lock, but returns 
* 	immediately if it cannot.
* 
* @param lock  The lock to try and lock.
* 
* @return 0 on success, a negative integer on failure.
*/
int tts_try_lock(tts_lock_t* lock)
{
	int flag = 0;
	atomic_xchg(&flag, &lock->lock);
	
	if(flag == 1)
	{
		//We got the lock.
		return 0;
	}
	else
	{
		//We failed to get the lock.
		return -1;
	}
}


/** 
* @brief Locks a basic test and test-and-set lock.
* 
* @param lock The lock to lock.
* 
* @return Zero on success.
*/
int tts_lock(tts_lock_t* lock)
{
	int flag = 0;

	//Just spin.
	while(flag == 0)
	{
		//If we have a chance of acquiring the lock...
		if(lock->lock > 0)
		{
			atomic_xchg(&flag, &lock->lock);
		}
		else
		{
			yield(-1);
		}
	}
	
	return 0;
}

/** 
* @brief Unlocks a basic test and test-and-set lock.
* 
* @param lock The lock to unlock.
* 
* @return Zero on success, a negative int when the lock
* 	isn't locked to begin with.
*/
int tts_unlock(tts_lock_t* lock)
{
	int value = 1;
	atomic_xchg(&value, &lock->lock);

	//The lock wasn't unlocked to begin with.
	if(value != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/** 
* @brief Initializes a basic test and test-and-set lock.
* 
* @param lock The lock to initialize.
* 
* @return Zero on success, a negative in otherwise.
*/
int tts_init(tts_lock_t* lock)
{
	lock->lock = 1;
	lock->tid = 0;
	return 0;
}

/** 
* @brief Destroys a basic test and test-and-set lock.
* 
* @param lock The lock to destroy.
* 
* @return Zero on success, a negative int, if the lock 
* 	was being used.
*/
int tts_destroy(tts_lock_t* lock)
{
	if(lock->lock == 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}


