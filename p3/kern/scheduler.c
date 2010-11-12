/** 
* @file scheduler.c
* @brief Scheduler functions for the 410 kernel. 
* @author Justin Scheiner
* @author Tim Wilson
*/
#include <scheduler.h>
#include <context_switch.h>
#include <thread.h>
#include <list.h>
#include <simics.h>
#include <asm_helper.h>
#include <cr.h>
#include <types.h>
#include <timer.h>
#include <loader.h>
#include <string.h>
#include <heap.h>
#include <global_thread.h>
#include <debug.h>
#include <mutex.h>
#include <lifecycle.h>
#include <malloc.h>
#include <ecodes.h>

#define INIT_PROGRAM "init"

/**
 * @brief Circular queue of runnable threads. 
 *  Always points to a runnable thread if there is one, often but not
 *  always our current thread. 
 */
static tcb_t *runnable = NULL;

/**
 * @brief Circular queue of descheduled threads.
 *
 * If at any point there are no runnable or blocked threads, all
 * descheduled threads should be killed. This should never happen though,
 * since init will presumably always be runnable or blocked.
 */
static tcb_t *descheduled = NULL;

/** 
* @brief A min-heap keying on the time the sleeper will run next.
*/
static sleep_heap_t sleepers;

/**
 * @brief A mutual exclusion lock ensuring that only one thread at a time
 * tries to double the size of the sleep heap.
 */
static mutex_t sleep_double_lock;

/**
 * @brief The number of blocked_threads.
 */
static int blocked_count = 0;

/** 
* @brief Initialize the scheduler.
*/
void scheduler_init()
{
   heap_init(&sleepers);
	mutex_init(&sleep_double_lock);
   LIST_INIT_EMPTY(runnable);   
}

/**
 * @brief Register a thread as runnable. This should only be called once
 * per thread.
 *
 * @param tcb The tcb of the newly runnable thread.
 */
void scheduler_register(tcb_t* tcb)
{
   debug_print("scheduler", "Adding %p to the scheduler with tid 0x%x", 
			tcb, tcb->tid);
   
   quick_lock();
   LIST_INSERT_BEFORE(runnable, tcb, scheduler_node);
   quick_unlock();
}

/**
 * @brief Run the thread with the given tcb
 *
 * @param tcb The tcb of the thread to run.
 * @param lock A lock preventing tcb from becoming invalid before we start
 * running it.
 *
 * @return True if the thread was successfully run, false if the target
 * thread was descheduled or blocked.
 */
boolean_t scheduler_run(tcb_t* tcb, mutex_t *lock)
{
	quick_lock();
	mutex_unlock(lock);
	if (tcb->descheduled || tcb->blocked)
   {
      quick_unlock();
		return FALSE;
   }
	LIST_REMOVE(runnable, tcb, scheduler_node);
	LIST_INSERT_AFTER(runnable, tcb, scheduler_node);
	scheduler_next();
	return TRUE;
}

/**
 * @brief Remove ourself from the runnable list to avoid being scheduled and
 * run the next thread. Interrupts must be disabled, otherwise we could be
 * in a situation where we start blocking, someone tries to unblock us,
 * and then we block forever.
 */
void scheduler_block()
{
	quick_assert_locked();
	tcb_t *tcb = get_tcb();
	debug_print("scheduler", "Blocking myself, thread %p", tcb);
	blocked_count++;
	tcb->blocked = TRUE;
	LIST_REMOVE(runnable, tcb, scheduler_node);
	scheduler_next();
}

/**
 * @brief Mark the given thread as unblocked and schedule them if they are
 * not descheduled as well. 
 *
 * @param tcb The thread to unblock.
 */
void scheduler_unblock(tcb_t* tcb)
{
	debug_print("scheduler", "Unblocking thread %p", tcb);
	assert(tcb->blocked);
	quick_lock();
	blocked_count--;
	tcb->blocked = FALSE;
	if (!tcb->descheduled && tcb->wakeup == 0) {
		LIST_INSERT_AFTER(runnable, tcb, scheduler_node);
	}
	quick_unlock();
}

/**
 * @brief Deschedule ourself, preventing the kernel from running us until
 * someone reschedules us.
 *
 * @param lock The invoking thread's deschedule lock to prevent a
 * make_runnable call from preempting us.
 */
void scheduler_deschedule(mutex_t *lock)
{
	tcb_t *tcb = get_tcb();
	debug_print("scheduler", "Descheduling thread %p", tcb);
	quick_lock();
	mutex_unlock(lock);
	assert(!tcb->descheduled);
	tcb->descheduled = TRUE;
	LIST_REMOVE(runnable, tcb, scheduler_node);
	scheduler_next();
}

/**
 * @brief Reschedule a thread after a call to deschedule
 *
 * @param tcb The thread to reschedule
 *
 * @return True if the thread was successfully rescheduled, false if the
 * thread was already scheduled.
 */
boolean_t scheduler_reschedule(tcb_t *tcb)
{
	debug_print("make_runnable", "Rescheduling thread %p", tcb);
	quick_lock();
	if (tcb->descheduled) {
		tcb->descheduled = FALSE;
		debug_print("make_runnable", "Marking %p not descheduled", tcb);
		if (!tcb->blocked && tcb->wakeup == 0) {
			LIST_INSERT_BEFORE(runnable, tcb, scheduler_node);
			debug_print("make_runnable", "Adding %p to scheduler", tcb);
		}
		quick_unlock();
		return TRUE;
	}
	quick_unlock();
	return FALSE;
}

/**
 * @brief Remove ourself from the runnable queue, ensuring that we never
 * run again.
 *
 * @param A locked mutex we're holding before we die. Prevents our stack
 * from being freed before we finish.
 */
void scheduler_die(mutex_t *lock)
{
	tcb_t *tcb = get_tcb();
	debug_print("scheduler", "Dying %p", tcb);
	quick_lock();
	mutex_unlock(lock);
	LIST_REMOVE(runnable, tcb, scheduler_node);
	scheduler_next();
	assert(FALSE);
}

/**
 * @brief Switch to the next thread in the runnable queue.
 *    This function should never be called with interrupts enabled!!!!!!
 */
void scheduler_next()
{
	quick_assert_locked();
	tcb_t *tcb = get_tcb();
   tcb_t *sleeper;
   unsigned long now;
   //debug_print("scheduler", "scheduler_next, old tcb = %p", tcb);
   sleeper = heap_peek(&sleepers); 
   now = time();
   
   /* If it is time to wake up a thread, put him last in the run queue. */
   if(sleeper && sleeper->wakeup < now)
   {
		debug_print("sleep", "Waking tcb %p from %p", sleeper, tcb);
      heap_pop(&sleepers);
		sleeper->wakeup = 0;
      LIST_INSERT_BEFORE(runnable, sleeper, scheduler_node);
      runnable = sleeper;
   }

   if(!runnable)
   {
      /* There is a sleeping or blocked thread, and no one to run - 
		 * twiddle our thumbs.*/
      if(sleeper || blocked_count > 0)
      {
			tcb_t *next	= global_tcb();
			//debug_print("scheduler", "Now running global thread %p", next);
   		set_esp0((int)next->kstack);

         assert(next->dir_p);
			quick_fake_unlock();
   		context_switch(&tcb->esp, &next->esp, next->dir_p);
			quick_unlock_all();
			return;
      }
		else {
	      /* If there is no one in the run queue, we are responsible 
   	    * for launching the first task (again if necessary). Presumably
			 * this will never happen since init should always be runnable or
			 * blocked.
      	 */
			debug_print("scheduler", "reloading init");
			tcb_t *killed;
			LIST_FORALL(descheduled, killed, scheduler_node) {
				thread_kill("No possibility of rescheduling");
			}
   	   load_new_task(INIT_PROGRAM, 1, INIT_PROGRAM, strlen(INIT_PROGRAM) + 1);
			assert(FALSE);
   	}
	}
	
	runnable = LIST_NEXT(runnable, scheduler_node);
	debug_print("scheduler", "now running %p", runnable);
   set_esp0((int)runnable->kstack);
   assert(runnable->dir_p);
	quick_fake_unlock();
   context_switch(&tcb->esp, &runnable->esp, runnable->dir_p);
	quick_unlock_all();
}

/**
 * @brief Put the calling thread to sleep for the given time. 
 *    
 *    As of now - only this function calls heap_insert, and only 
 *    scheduler_next calls heap_peek / heap_pop. heap_remove will
 *    need to be called by dying processes with interrupts disabled. 
 *
 * @param ticks The number of timer ticks to sleep for.
 *
 * @return ESUCCESS on successful sleep
 *         EFAIL if the thread could not be successfully put to sleep
 */
int scheduler_sleep(unsigned long ticks)
{
   tcb_t* tcb = get_tcb();
   debug_print("sleep", "%p going to sleep for %d ticks", tcb, ticks);
	mutex_lock(&sleep_double_lock);

	/* Double the sleep heap if necessary. */
	if (heap_check_size(&sleepers) == ESUCCESS) {
		/* Atomically insert into the sleep heap after possibly doubling
		 * the heap. */
	   quick_lock();
		mutex_unlock(&sleep_double_lock);
   	tcb->wakeup = time() + ticks;
   	heap_insert(&sleepers, tcb);
   	LIST_REMOVE(runnable, tcb, scheduler_node);
   	scheduler_next();
		return ESUCCESS;
	}
	else {
		/* We needed to double the sleep heap but ran out of memory. */
		mutex_unlock(&sleep_double_lock);
		return EFAIL;
	}
}


