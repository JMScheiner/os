
#include <mutex_type.h>
#include <thr_internals.h>
#include <atomic.h>

int mutex_init(mutex_t *mp)
{
	//mp->next = NULL
	//mp->start = NULL
}

int mutex_destroy(mutex_t *mp)
{

}


//When I say "list end modifier lock" 
//	I'm referring to a lock we'll put in each 
//		thread's space (a simple TS lock that we can spin on).
//		For each of these locks, there are only two threads touching
//		them, the one adding itself to the list, and the one that
//		owns the lock.

int mutex_lock( mutex_t *mp )
{
	// Retrieve tcb for this thread.
	// Exchange mp->next with my own list* 
	// 	^^ This line ensures that only one thread
	// 		sees a given node as the end of the list.
	//
	// if(list != NULL)
	// {
	// 	Lock list end modifier lock, or spin.
	//
	// 	if(mp->start)
	// 	{
	// 		//The guy hasn't released his lock yet.
	//			list->next = me;
	//			deschedule()
	//		}
	//		else
	//		{
	//			//The guy has already released the lock.
	//			mp->start = me;
	//			free to run (just return).
	//		}
	//		Unlock list end modifier lock.
	// }
	// else
	// {
	// 	mp->start = me
	//		We're free to run.
	// }
	//		
}

int mutex_unlock( mutex_t *mp )
{
	// Lock list end modifier lock, or spin.
	// if(my->next != NULL)
	// {
	// 	set my->next runnable.
	// }
	// else
	// {
	// 	set mp->start = NULL
	// }
	// continue running
	//
	// Unlock list end modifier lock.
}



