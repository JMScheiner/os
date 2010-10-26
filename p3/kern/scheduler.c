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
#include <x86/asm.h>
#include <simics.h>
#include <asm_helper.h>
#include <cr.h>
#include <types.h>
#include <timer.h>
#include <loader.h>
#include <string.h>

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
 * @brief Initialize the scheduler queues.
 */
void scheduler_init()
{
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
 */
void scheduler_next()
{
   tcb_t* current = runnable;
   
   if(!runnable)
   {
      /* If there is no one in the run queue, we are responsible 
       * for launching the first task (again if necessary).
       */
      load_new_task(INIT_PROGRAM, 1, INIT_PROGRAM, strlen(INIT_PROGRAM) + 1);
   }

   disable_interrupts();
   runnable = LIST_NEXT(runnable, scheduler_node);
   set_esp0((int)runnable->kstack);
   context_switch(&current->esp, &runnable->esp);
   enable_interrupts();
}

/**
 * @brief Put a thread to sleep for at least a certain period of time.
 *
 * @param tcb The tcb of the thread to put to sleep.
 * @param ticks The number of (timer ticks?, milliseconds?, probably should be
 * milliseconds) to sleep for.
 */
void scheduler_sleep(tcb_t* tcb, unsigned long ticks)
{
   //TODO
}


