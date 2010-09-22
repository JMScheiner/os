/** 
* @file mutex.c
* @brief See function descriptions.
* @author Justin Scheiner
*/

#include <mutex_type.h>
#include <thr_internals.h>
#include <syscall.h>
#include <atomic.h>
#include <assert.h>
#include <stddef.h>
#include <simics.h>
#include <stdio.h>

/** 
* @brief Initialize a mutex for locking.
* 
* @param mp The mutex to initialize.
* 
* @return 0 on success, -1 on failure.
*/
int mutex_init(mutex_t *mp)
{
	if(!mp)
		return -1;

	mp->last = NULL;
	mp->running = NULL;
	mp->next = NULL;
	mp->held = 0;
	mp->tid = 0;
	mp->initialized = 1;
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
	if(!mp)
		return -1;
	
	//FIXME mutex_destroy needs to atomically 
	//	deactivate the mutex, so we can block 
	//	invocations of lock_mutex, and return -1 if someone
	//	holds the mutex illegally.
	
	mp->initialized = 0;
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


/** 
* @brief Locks a mutex.
* 
* @param mp The mutex to lock.
*
* 	This is complicated, but everything is serialized by atomically
* 		swapping yourself into the end of the list.
*	
*	Each list element contains a basic lock which it contends for
*		with the next thread that will call mutex_lock.
*		
*		- Threads "try_lock" the lock in the parent.
*			
*			- If they succeed in acquiring their parents lock, they can safely
*				tell the parent who they are, to prepare for descheduling.
*
*				- The parent is able to cancel deschedule by setting cancel_deschedule in the child,
*					so there isn't a race condition between deschedule and make_runnable.
*			
*			- If they fail to acquire the lock, they busy wait for him to finish
*				on the value "held" in the mutex.
*			
*		- When parents acquire the lock, they set mp->next to either NULL or their child.
*			
*			- This is so in mutex_unlock we can decide whether to take down the "held" flag 
*				and let children who failed to get the lock go, or actively "make_runnable" 
*				the next person waiting for the mutex.
*		
*		- Every thread is blocking on mutex->held, busy waiting on its parent, or is running in the 
*			critical section.
* 
* @return 0 on success, 
* 	-1 on failure.
*/
int mutex_lock( mutex_t *mp )
{
	if(!mp)
		return -1;
	
	if(!mp->initialized)
		return -2;
	
	mutex_node me;
	mutex_node* swap = &me;
	
	//Initialize myself. 
	me.tid = gettid();
	me.next = NULL;
	tts_init(&me.access);
	
	//Swap yourself into the end of the mutex's list.
	//  This is the line that guarantees bounded waiting.
	atomic_xchg((int*)&swap, (int*)&mp->last);
	
	if(swap)
	{
		//Protect against accessing something that is no longer on the stack:
		int accessible = tts_try_lock(&swap->access);
		
		if(accessible == 0)
		{
			//We are free to modify the guy before us.
			swap->next = &me;
			me.cancel_deschedule = 0;
			
			//Open the flood gates - this works because 
			//	(1), (2), and (3) are valid in any order.
			tts_unlock(&swap->access);
			deschedule(&me.cancel_deschedule); /* (1) */
		}
		else
		{
			//We tried to lock but "swap" was past (4)
			//We are next to run.
			
			// What if the other guy runs ALL the way through
			// 	before we get to this point? 
			// 		- Everyone after us is blocking above, 
			// 		- mp->running is still 0. (who would have changed it?)
			while(mp->held)
				yield(mp->tid);
		}
	}
	
	/************* Lock Acquired **************/

	mp->held = 1;
	mp->tid = me.tid;

	//This, "running" will only serve as an identifier to insure that 
	// someone has modified the list by the time we finish. It will never
	// actually be accessed.
	mp->running = &me;
	
	//Protect against accessing "me" after this point:
	
	tts_lock(&me.access); /* (4) */
	//FIXME ^^ This lock isn't magic. It NEEDS to be somewhere
	//	safe and the stack _isn't_ safe.  I could see the call
	//	to tts_try_lock above succeeding errantly, which would
	//		be a disaster!!!!
	
	mp->next = me.next;
	return 0;
}

/** 
* @brief Unlock the mutex.
*
* Following on the description of mutex_lock, after its critical section,
* 	a thread is responsible for:
* 		- Releasing a busy waiting thread they don't know about, or
* 		- make_runnable'ing a thread they do know about. "...if(next)..."
*
*	I would like to cmpxchg NULL into "mp->last" if possible, but this isn't
*		necessary for unlock to be correct.
* 
* @param mp  The mutex to unlock.
* 
* @return 0 on success.
* 	-1 on failure.
*/
int mutex_unlock( mutex_t *mp )
{
	assert(mp);
	assert(mp->initialized);

	mutex_node* next = mp->next;
	if(next)
	{
		//Someone is trying to run. Let him go.
		next->cancel_deschedule = 1; /* (2) */
		make_runnable(next->tid);    /* (3) */
	}
	else
	{
		//Let a thread that wasn't able to tell this thread
		// about itself go.
		mp->held = 0;
	}
	
	//If after all that we are still last, we can safely swap in NULL.
	//next = NULL;
	
	//TODO This would be nice, but simics is complaining about cmpxchg.
	//	 It is not necessary.
	//atomic_cmpxchg((int*)&next, (int*)&mp->last, (int)mp->running);
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
		return 0;
	}
	else
	{
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
		//TODO Yield to tid here? 
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


