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
#include <syscall.h>
#include <cond_type.h>
#include <mutex.h>
#include <thr_internals.h>

int cond_init( cond_t* cv)
{
	mutex_init(&cv->qlock);
	STATIC_INIT_QUEUE(&cv->q);
	return 0;
}

int cond_destroy( cond_t* cv)
{
	mutex_destroy(&cv->qlock);
	return 0;
}

int cond_wait( cond_t* cv, mutex_t* mp )
{
	mutex_lock(&cv->qlock);

	cond_link_t link;
	link.tid = gettid();
	link.cancel_deschedule = FALSE;
	
	mutex_lock(&cv->qlock);
	ENQUEUE_LAST(&cv->q, &link);
	mutex_unlock(&cv->qlock);
	
	deschedule((int*)&link.cancel_deschedule);

	//I am not sure that I am using mp right here.
	mutex_lock(mp);

	return 0;
}

int cond_signal( cond_t* cv )
{
	cond_link_t* link;
	
	mutex_lock(&cv->qlock);
	
	DEQUEUE_FIRST(&cv->q, link);
	if(link)
	{
		link->cancel_deschedule = TRUE;
		make_runnable(link->tid);
	}
	
	mutex_unlock(&cv->qlock);
	
	return 0;
}

int cond_broadcast( cond_t* cv)
{
	cond_link_t* link;

	mutex_lock(&cv->qlock);
	FOREACH(&cv->q, link)
	{
		link->cancel_deschedule = TRUE;
		make_runnable(link->tid);
	}
	mutex_unlock(&cv->qlock);
	
	return 0;
}


