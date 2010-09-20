
#include <mutex_type.h>
#include <thr_internals.h>
#include <syscall.h>
#include <atomic.h>

tcb_t* get_tcb()
{
	return NULL;
}

int mutex_init(mutex_t *mp)
{
	STATIC_INIT_QUEUE(&mp->q);
	tts_init(&mp->qlock);
	return 0;
}

int mutex_destroy(mutex_t *mp)
{
	return 0;		
}

int mutex_lock( mutex_t *mp )
{
	int value = 1;
	tcb_t* next, *me;
	
	me = get_tcb();
	
	tts_lock(&mp->qlock);
	PEEK_FIRST(&mp->q, next);

	if(next)
	{
		ENQUEUE_LAST(&mp->q, me);
		tts_unlock(&mp->qlock);

		//BANG Bad stuff happens here.
		//	Specifically, we get make_runnable'd and never wake up again.
		//	Need to xchg _something_ here.
		
		deschedule(&value);
	}
	return 0;
}

int mutex_unlock( mutex_t *mp )
{
	tcb_t* next;
	tts_lock(&mp->qlock);
	
	DEQUEUE_FIRST(&mp->q, next);
	if(next != 0)
	{
		//Need some extra machinery to see if the person
		//	is ready to be made runnable.
		make_runnable(next->tid);
	}

	tts_unlock(&mp->qlock);
	return 0;
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


