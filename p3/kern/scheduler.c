/** 
* @file scheduler.c
* @brief Scheduler functions for the 410 kernel. 
* @author Justin Scheiner
* @author Tim Wilson
* @bug Should sleepers always be put at the front of the run queue when they wake up?
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

#define INIT_PROGRAM "init"

/**
 * @brief Circular queue of runnable threads. 
 *  Always points to the next runnable thread, if there is one. 
 */
static tcb_t *runnable = NULL;

/**
 * @brief Circular queue of descheduled threads.
 *
 * If at any point there are no runnable or blocked threads, all
 * descheduled threads should be killed.
 */
static tcb_t *descheduled = NULL;

/** 
* @brief A min-heap keying on the time the sleeper will run next.
*/
static sleep_heap_t sleepers;

static int blocked_count = 0;

/** 
* @brief Initialize the scheduler.
*/
void scheduler_init()
{
   heap_init(&sleepers);
   LIST_INIT_EMPTY(runnable);   
}

/**
 * @brief Register a thread as runnable.
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
 * Interrupts must always be disabled before calling this
 * function, otherwise we may accidently try to run a zombie thread.
 *
 * @param tcb The tcb of the thread to run.
 */
void scheduler_run(tcb_t* tcb)
{
	assert(tcb->blocked == FALSE);
	assert(tcb->descheduled == FALSE);
	LIST_REMOVE(runnable, tcb, scheduler_node);
	LIST_INSERT_BEFORE(runnable, tcb, scheduler_node);
	scheduler_next();
}

/**
 * @brief Remove ourself from the runnable list to avoid being scheduled and
 * run the next thread.
 */
void scheduler_block()
{
	tcb_t *tcb = get_tcb();
	debug_print("scheduler", "Blocking myself, thread %p", tcb);
	quick_lock();
	blocked_count++;
	tcb->blocked = TRUE;
	LIST_REMOVE(runnable, tcb, scheduler_node);
	scheduler_next();
}

/**
 * @brief Mark the given thread as unblocked and schedule them if they are
 * not descheduled. 
 *
 * @param tcb The thread to unblock.
 */
void scheduler_unblock(tcb_t* tcb)
{
	debug_print("scheduler", "Unblocking thread %p", tcb);
	quick_lock();
	assert(tcb->blocked);
	blocked_count--;
	tcb->blocked = FALSE;
	if (!tcb->descheduled && tcb->wakeup == 0) {
		LIST_INSERT_BEFORE(runnable, tcb, scheduler_node);
	}
	quick_unlock();
}

/**
 * @brief Deschedule ourself, preventing the kernel from running us until
 * someone reschedules us.
 */
void scheduler_deschedule()
{
	tcb_t *tcb = get_tcb();
	debug_print("scheduler", "Descheduling thread %p", tcb);
	quick_lock();
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
	debug_print("scheduler", "Rescheduling thread %p", tcb);
	quick_lock();
	if (tcb->descheduled) {
		tcb->descheduled = FALSE;
		if (!tcb->blocked && tcb->wakeup == 0)
			LIST_INSERT_BEFORE(runnable, tcb, scheduler_node);
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
 * @param A locked mutex we're holding before we die.
 * @param A pcb to free if we are the last thread in our process, NULL
 * otherwise.
 */
void scheduler_die(mutex_t *lock, pcb_t *pcb)
{
	tcb_t *tcb = get_tcb();
	debug_print("scheduler", "Dying %p", tcb);
	quick_lock();
	mutex_unlock(lock);
	if (pcb != NULL)
		sfree(pcb, sizeof(pcb_t));
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
	tcb_t *tcb = get_tcb();
   tcb_t *sleeper;
   unsigned long now;
   //debug_print("scheduler", "scheduler_next, old tcb = %p", tcb);
   sleeper = heap_peek(&sleepers); 
   now = time();
   
   /* If it is time to wake up a thread, put him first in the run queue. 
    *
    *  Is this a good policy? I can see people doing sleep(1) all of the 
    *  time, and causing starvation.
    **/
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
   		context_switch(&tcb->esp, &next->esp, next->pcb->dir_p);
			quick_unlock_all();
			return;
      }
		else {
	      /* If there is no one in the run queue, we are responsible 
   	    * for launching the first task (again if necessary).
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
   context_switch(&tcb->esp, &runnable->esp, runnable->pcb->dir_p);
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
 */
void scheduler_sleep(unsigned long ticks)
{
   tcb_t* tcb = get_tcb();
   debug_print("sleep", "%p going to sleep for %d ticks", tcb, ticks);
   tcb->wakeup = time() + ticks;
   
   quick_lock();
   heap_insert(&sleepers, tcb);
   LIST_REMOVE(runnable, tcb, scheduler_node);
   scheduler_next();
}


