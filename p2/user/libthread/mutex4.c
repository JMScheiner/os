#include <mutex_type.h>
#include <thr_internals.h>
#include <syscall.h>
#include <atomic.h>

int mutex_init(mutex_t *mp)
{

}

int mutex_destroy(mutex_t *mp)
{

}

int mutex_try_lock(mutex_t *mp)
{

}


int mutex_lock( mutex_t *mp )
{
	mutex_node me;
	mutex_node* swap = &me;
	
	//Initialize myself. 
	me.tid = gettid();
	me.next = NULL;
	tts_init(&me.access);
	
	//Swap yourself into the end of the mutex's list.
	//  This is the line that guarantees bounded waiting.
	atomic_xchg(&swap, &mp->last);
	
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
		deschedule(&me->cancel_deschedule); /* (1) */
	}
	else
	{
		//We tried to lock but "swap" was past (4)
		//We are next to run.
		while(mp->running)
			yield(mp->active_tid);
	}
	
	/************* Lock Acquired **************/

	mp->running = 1;
	mp->active_tid = me.tid;
	
	//Protect against accessing "me" after this point:
	
	tts_lock(&me.access); /* (4) */
	//FIXME ^^ This lock isn't magic. It NEEDS to be somewhere
	//	safe and the stack _isn't_ safe.  I could see the call
	//	to tts_try_lock above succeeding errantly, which would
	//		be a disaster.
	
	mp->next = me.next;
	return 0;
}

int mutex_unlock( mutex_t *mp )
{
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
		mp->running = 0;
	}
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
	atomic_xchg(&flag, lock);
	
	if(flag == 1)
		return 0;
	else
		return -1;
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
		if(*lock > 0)
		{
			atomic_xchg(&flag, lock);
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
	atomic_xchg(lock, &value);

	//The lock wasn't unlocked to begin with.
	if(value != 0)
		return -1;
	else
		return 0;
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
	*lock = 1;
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
	if(*lock == 0)
		return -1;
	
	return 0;
}


