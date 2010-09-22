/** 
* @file cond.c
* @brief See function descriptions.
* 	Generally though, this relies on _our_ implementation of
* 	mutexes, for example, 
* 		- It is safe to call mutex_unlock when you don't "own" the mutex.
* 		- Mutex's are fair, and have bounded waiting.
*
* @author Justin Scheiner
* @date 2010-09-21
*/

#include <cond.h>
#include <mutex.h>

int cond_init( cond_t* cv)
{
	mutex_init(cv->lock);
	mutex_lock(cv->lock);
	return 0;
}

int cond_destroy( cond_t* cv)
{
	mutex_destroy(cv->lock);
	return 0;
}

int cond_wait( cond_t* cv, mutex_t* mp)
{
	int nthreads = 1;

	//FIXME I assume here that cond_wait WON'T be called with a different 
	//	mutex.  I'm not sure this is a good assumption, but the docs say 
	//
	//	"release the associated mutex that it needs to hold to check the condition"
	//
	cv->lock = mp;
	atomic_xadd(&nthreads, &cv->nthreads);

	//Acquire the lock for the condition variable.
	//	Note that only a cond_signal or cond_broadcast will release the lock
	//	so this is fine.
	mutex_lock(cv->lock);
	
	nthreads = -1;
	atomic_xadd(&nthreads, &cv->nthreads);
	
	// The last thread shouldn't release the mutex.
	if(cv->broadcast_countdown > 1 && cv->broadcast)
	{
		cv->broadcast_countdown--;
		mutex_unlock(cv->lock);
	}
	else if(cv->broadcast_countdown == 1)
	{
		cv->broadcast_countdown = 0;
		cv->broadcast = 0;
	}

	return 0;
}

int cond_signal( cond_t* cv )
{
	//If we beat the waiting thread to its call to mutex_lock, that's fine, 
	//	since it will be a race condition either way, and it is consistent with the spec.
	
	if(cv->nthreads)
		mutex_unlock(cv->lock);	
	
	return 0;
}

int cond_broadcast( cond_t* cv)
{
	cv->broadcast = 1;
	cv->broadcast_countdown = cv->nthreads;

	if(cv->broadcast_countdown)
		mutex_unlock(cv->lock);
	return 0;
}


