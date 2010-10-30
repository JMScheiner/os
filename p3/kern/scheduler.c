/** 
* @file scheduler.c
* @brief Scheduler functions for the 410 kernel. 
* @author Justin Scheiner
* @author Tim Wilson
* @bug Should sleepers always be put at the front of the run queue when they wake up?
* @bug Will launch a new task if every current thread is sleeping. This is undoubtedly
*  the WRONG behavior - but I haven't worked out what is right yet.
*/
#include <scheduler.h>
#include <context_switch.h>
#include <thread.h>
#include <list.h>
#include <x86/asm.h>
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

#define INIT_PROGRAM "sleep"

/**
 * @brief Circular queue of runnable threads. 
 *  Always points to the next runnable thread, if there is one. 
 */
static tcb_t* runnable;

/**
 * @brief Circular queue of blocked threads.
 */
static tcb_t* blocked;

/** 
* @brief A min-heap keying on the time the sleeper will run next.
*/
static sleep_heap_t sleepers;

/** 
* @brief Initialize the scheduler.
*/
void scheduler_init()
{
   heap_init(&sleepers);
   INIT_LIST(runnable);   
   INIT_LIST(blocked); 
}

/**
 * @brief Register a thread as runnable.
 *
 * Note: This thread must not be in the runnable queue before
 * calling this function.
 *
 * @param tcb The tcb of the newly runnable thread.
 */
void scheduler_register(tcb_t* tcb)
{
   debug_print("scheduler", "Adding %p to the scheduler with tid 0x%x", tcb, tcb->tid);
   LIST_INIT_NODE(tcb, scheduler_node);
   
   disable_interrupts();
   LIST_INSERT_BEFORE(runnable, tcb, scheduler_node);
   enable_interrupts();
}

/**
 * @brief Run the thread with the given tcb
 *
 * Note: I believe interrupts should always be disabled before calling this
 * function, otherwise we may accidently try to run a zombie thread.
 *
 * @param tcb The tcb of the thread to run.
 */
void scheduler_run(tcb_t* tcb)
{
	disable_interrupts();
	LIST_REMOVE(runnable, tcb, scheduler_node);
	LIST_INSERT_BEFORE(runnable, tcb, scheduler_node);
	scheduler_next(get_tcb());
	enable_interrupts();
}

void scheduler_make_runnable(tcb_t* tcb)
{
	debug_print("scheduler", "Unblocking thread %p", tcb);
	disable_interrupts();
	LIST_REMOVE(blocked, tcb, scheduler_node);
	LIST_INSERT_BEFORE(runnable, tcb, scheduler_node);
	enable_interrupts();
}

/**
 * @brief Move a thread from the runnable queue to the blocked queue.
 *
 * @param tcb The thread to move to the blocked queue.
 */
void scheduler_block(tcb_t* tcb)
{
	debug_print("scheduler", "Blocking thread %p", tcb);
	disable_interrupts();
	LIST_REMOVE(runnable, tcb, scheduler_node);
	LIST_INSERT_BEFORE(blocked, tcb, scheduler_node);
	enable_interrupts();
}

/**
 * @brief Place ourself on the blocked list to avoid being scheduled and
 * run the next thread.
 */
void scheduler_block_me()
{
	tcb_t *tcb = get_tcb();
	debug_print("scheduler", "Blocking myself, thread %p", tcb);
	disable_interrupts();
	LIST_REMOVE(runnable, tcb, scheduler_node);
	LIST_INSERT_BEFORE(blocked, tcb, scheduler_node);
	scheduler_next(tcb);
	enable_interrupts();
}

/**
 * @brief Remove ourself from the runnable queue, ensuring that we never
 * run again.
 *
 * @param A locked mutex we're holding before we die.
 */
void scheduler_die(mutex_t *lock)
{
	debug_print("scheduler", "Dying %p", get_tcb());
	disable_interrupts();
	mutex_unlock(lock);
	LIST_REMOVE(runnable, get_tcb(), scheduler_node);
	scheduler_next(NULL);
	assert(FALSE);
}

/**
 * @brief Switch to the next thread in the runnable queue.
 *    This function should never be called with interrupts enabled!!!!!!
 */
void scheduler_next(tcb_t* tcb)
{
   tcb_t *sleeper;
   unsigned long now;
   debug_print("scheduler", "scheduler_next, old tcb = %p", tcb);
   sleeper = heap_peek(&sleepers); 
   now = time();
   
   /* If it is time to wake up a thread, put him first in the run queue. 
    *
    *  Is this a good policy? I can see people doing sleep(1) all of the time, 
    *    and causing starvation.
    **/
   if(sleeper && sleeper->wakeup < now)
   {
		 debug_print("sleep", "Waking tcb %p from %p", sleeper, tcb);
      heap_pop(&sleepers);
      LIST_INSERT_BEFORE(runnable, sleeper, scheduler_node);
      runnable = sleeper;
   }

   if(!runnable)
   {
      /* There is a sleeping thread, and no one to run - twiddle our thumbs.*/
      if(sleeper || blocked)
      {
         tcb_t* global = global_tcb();
				 debug_print("scheduler", "running global thread %p", global);
         set_esp0((int)global_tcb()->kstack);
         context_switch(&tcb->esp, 
            &global->esp, global->pcb->dir_p);
          return;
      }
      
      /* If there is no one in the run queue, we are responsible 
       * for launching the first task (again if necessary).
       */
			debug_print("scheduler", "reloading init");
      load_new_task(INIT_PROGRAM, 1, INIT_PROGRAM, strlen(INIT_PROGRAM) + 1);
   }
   runnable = LIST_NEXT(runnable, scheduler_node);
	 debug_print("scheduler", "now running %p", runnable);
   set_esp0((int)runnable->kstack);
   context_switch(&tcb->esp, &runnable->esp, runnable->pcb->dir_p);
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
   tcb_t* me = get_tcb();
   debug_print("sleep", "%p going to sleep for %d ticks", me, ticks);
   me->wakeup = time() + ticks;
   
   disable_interrupts();
   heap_insert(&sleepers, me);
   LIST_REMOVE(runnable, me, scheduler_node);
   scheduler_next(me);
   enable_interrupts();
}


