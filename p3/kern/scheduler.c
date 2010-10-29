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

#define INIT_PROGRAM "readline_basic"

/**
 * @brief Circular queue of runnable threads.
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
   //lprintf("Adding %x to the scheduler.", tcb->tid);
   LIST_INIT_NODE(tcb, scheduler_node);   
   
   disable_interrupts();
   LIST_INSERT_AFTER(runnable, tcb, scheduler_node);
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
	LIST_INSERT_AFTER(runnable, tcb, scheduler_node);
	scheduler_next();
   enable_interrupts();
}

void scheduler_make_runnable(tcb_t* tcb)
{
   disable_interrupts();
   LIST_REMOVE(blocked, tcb, scheduler_node);
   LIST_INSERT_AFTER(runnable, tcb, scheduler_node);
   enable_interrupts();

}

/**
 * @brief Move a thread from the runnable queue to the blocked queue.
 *
 * @param tcb The thread to move to the blocked queue.
 */
void scheduler_block(tcb_t* tcb)
{
   disable_interrupts();
   LIST_REMOVE(runnable, tcb, scheduler_node);
   LIST_INSERT_BEFORE(blocked, tcb, scheduler_node);
   enable_interrupts();
}

void scheduler_block_me(tcb_t* me)
{
   disable_interrupts();
   LIST_REMOVE(runnable, me, scheduler_node);
   LIST_INSERT_BEFORE(blocked, me, scheduler_node);
   scheduler_next();
   enable_interrupts();
}

/**
 * @brief Switch to the next thread in the runnable queue.
 *    This function should never be called with interrupts enabled!!!!!!
 */
void scheduler_next()
{
   tcb_t *current, *sleeper;
   unsigned long now;
   
   sleeper = heap_peek(&sleepers); 
   now = time();
   
   /* If it is time to wake up a thread, put him first in the run queue. 
    *
    *  Is this a good policy? I can see people doing sleep(1) all of the time, 
    *    and causing starvation.
    **/
   if(sleeper && sleeper->wakeup < now)
   {
      LIST_INSERT_BEFORE(runnable, sleeper, scheduler_node);
   }
   else if(sleeper && !runnable)
   {
      /* Do something useful here!!! */
   }

   current = runnable;
   if(!runnable)
   {
      if(blocked)
      {
         lprintf("Deadlock!");
         MAGIC_BREAK;
      }
      /* If there is no one in the run queue, we are responsible 
       * for launching the first task (again if necessary).
       */
      load_new_task(INIT_PROGRAM, 1, INIT_PROGRAM, strlen(INIT_PROGRAM) + 1);
   }
   
   runnable = LIST_NEXT(runnable, scheduler_node);
   set_esp0((int)runnable->kstack);
   context_switch(&current->esp, &runnable->esp, runnable->pcb->dir_p);
}

/**
 * @brief Put the calling thread to sleep for the given time. 
 *    
 *    As of now - only this function calls heap_insert, and only 
 *    scheduler_next calls heap_peek / heap_pop. heap_remove will
 *    need to be called by dying processes with interrupts disabled. 
 *
 * @param ticks The number of (timer ticks?, milliseconds?, probably should be
 * milliseconds) to sleep for.
 */
void scheduler_sleep(unsigned long ticks)
{
   tcb_t* me = get_tcb();
   me->wakeup = time() + ticks;
   
   disable_interrupts();
   heap_insert(&sleepers, me);
   LIST_REMOVE(runnable, me, scheduler_node);
   scheduler_next();
   enable_interrupts();
}


