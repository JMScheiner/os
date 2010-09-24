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
#include <thread.h>
#include <syscall.h>
#include <thr_internals.h>
#include <simics.h>

int cond_init( cond_t* cv)
{
	mutex_init(&cv->qlock);
	STATIC_INIT_QUEUE(cv->q);
	return 0;
}

int cond_destroy( cond_t* cv)
{
	mutex_destroy(&cv->qlock);
	return 0;
}

int cond_wait( cond_t* cv, mutex_t* mp )
{
	cond_link_t link;
	link.tid = thr_getid();
	link.cancel_deschedule = FALSE;
	
	mutex_lock(&cv->qlock);
	ENQUEUE_LAST(cv->q, &link);
	mutex_unlock(&cv->qlock);
	
	//lprintf("[%d] About to unlock condition mutex.\n", thr_getid());
	mutex_unlock(mp);
	//lprintf("[%d] Unlocked condition mutex.\n", thr_getid());
	deschedule((int*)&link.cancel_deschedule);
	//lprintf("[%d] Back from deschedule!\n", thr_getid());
	mutex_lock(mp);

	return 0;
}

int cond_signal( cond_t* cv )
{
	cond_link_t* link;
	//lprintf("[%d] About to lock condition queue mutex.\n", thr_getid());
	mutex_lock(&cv->qlock);
	//lprintf("[%d] Locked condition queue mutex.\n", thr_getid());
	DEQUEUE_FIRST(cv->q, link);
	//lprintf("[%d] Dequeue'd link %p from condition queue.\n", thr_getid(), link);
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
	FOREACH(cv->q, link)
	{
		link->cancel_deschedule = TRUE;
		make_runnable(link->tid);
	}

	//Sorry about the wrong semantics, but this
	// needs to happen. Maybe an "EMPTY_QUEUE" function
	// is in order.
	STATIC_INIT_QUEUE(cv->q);
	mutex_unlock(&cv->qlock);
	
	return 0;
}


