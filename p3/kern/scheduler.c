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

/**
 * @brief Move a thread from the blocked queue to the runnable queue.
 *
 * @param tcb The thread to move to the runnable queue.
 */
void scheduler_block(tcb_t* tcb)
{
   disable_interrupts();
   LIST_REMOVE(runnable, tcb, scheduler_node);
   LIST_INSERT_BEFORE(blocked, tcb, scheduler_node);
   enable_interrupts();
}

/**
 * @brief Switch to the next thread in the runnable queue.
 *
 * @return The tcb of the thread that is made to run.
 */
tcb_t* scheduler_next()
{
   tcb_t* current = runnable;
   
   disable_interrupts();
   runnable = LIST_NEXT(runnable, scheduler_node);

   set_esp0((int)runnable->kstack);
   MAGIC_BREAK;
   context_switch(&current->esp, runnable->esp);
   MAGIC_BREAK;
   enable_interrupts();

   return runnable;
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






