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

}

int mutex_destroy(mutex_t *mp)
{

}

int mutex_try_lock(mutex_t *mp)
{
}


int mutex_lock( mutex_t *mp )
{
	int nthreads = 1;
	tcb_t* swap = *me = get_tcb();
	
	//If these could be atomic together, that would be swell.
	{ 
		atomic_xadd(&nthreads, &mp->lock_sem);
		atomic_xchg(&swap, &mp->last);
	}

	if(nthreads == 0)
	{
		//No one is in the critical section, 
		//	we're free to take the mutex, AND we are at 
		//	the end of the list, insuring we'll have someone
		//	to run.
	}
	else
	{
		//We need to queue ourselves for execution.
		assert(swap);
		me->cancel_deschedule = 0;

		//May unleash the make_runnable flood gate: 
		swap->next = me;
		deschedule(&me->cancel_deschedule);
	}
	return 0;
}

int mutex_unlock( mutex_t *mp )
{
	int nthreads = -1;
	tcb_t* me = get_tcb();
	
	atomic_xadd(&nthreads, &mp->lock_sem);
	if(nthreads == 0)
	{
		//We're good to just leave.
	}
	else
	{
		while(me->next == NULL) { /* busy wait */ } 
		me->next.cancel_deschedule = 0;
		make_runnable(me->next.tid);
	}
	
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


