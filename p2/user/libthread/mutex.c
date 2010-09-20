
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
	mp->in_use = 0;
	STATIC_INIT_QUEUE(&mp->q);
	tts_init(&mp->qlock);
	return 0;
}

int mutex_destroy(mutex_t *mp)
{
	tts_destroy(&mp->qlock);
	return 0;		
}

int mutex_try_lock(mutex_t *mp)
{
	int flag = 0;
	atomic_xchg(&flag, &mp->in_use);
	if(flag == 0)
		return -1;
	else
		return 0;
}


int mutex_lock( mutex_t *mp )
{
	int flag = 0;
	
	tts_lock(&mp->qlock);
	atomic_xchg(&flag, &mp->in_use);
	
	if(flag == 0)
	{
		//The mutex is in use, so we yield to the more 
		// complicated list mechanism...
		tcb_t* me = get_tcb();
		
		//Add ourself to the list.
		ENQUEUE_LAST(&mp->q, me);
		
		//Arrange for us to be descheduleable.
		me->dont_deschedule = 0;
		
		//Unleash the floodgates.
		tts_unlock(&mp->qlock);
			
		//If dont_deschedule is 1, that means the releasing thread has or 
		//	will make_runnable us, so we shouldn't deschedule.
		deschedule(&me->dont_deschedule); /* (1) */
	} 
	else tts_unlock(&mp->qlock);
	
	//We have the lock.
	return 0;
}

int mutex_unlock( mutex_t *mp )
{
	tcb_t* next;
	tts_lock(&mp->qlock);
	
	DEQUEUE_FIRST(&mp->q, next);
	if(next != 0)
	{
		//The list is occupied, we should let someone run.
		
		next->dont_deschedule = 1; /* (2) */
		make_runnable(next->tid);  /* (3) */
		
		//Every possible ordering of (1), (2), and (3) should be sound.
	} else mp->in_use = 1;

	tts_unlock(&mp->qlock);
	return 0;
}

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


