/** 
* @file scheduler.c
* @brief Scheduler functions for the 410 kernel. 
* @author Justin Scheiner
* @author Tim Wilson
* @bug We should replace disabling and enabling interrupts with a flag 
*  that blocks timer interrupts, or locks the scheduler.
*     - There's no reason to block all interrupts, when the only one that
*        will touch the scheduler is the timer.  Is the timer maskable? 
*/
#include <scheduler.h>
#include <context_switch.h>
#include <thread.h>
#include <list.h>
#include <x86/asm.h>
#include <simics.h>

static tcb_t* running;
static tcb_t* blocked;

void scheduler_init()
{
   INIT_LIST(running);   
   INIT_LIST(blocked); 
}

void scheduler_register(tcb_t* tcb)
{
   lprintf("Adding %x to the scheduler.", tcb->tid);
   LIST_INIT_NODE(tcb, scheduler_node);   
   
   disable_interrupts();
   LIST_INSERT_AFTER(running, tcb, scheduler_node);
   enable_interrupts();
}

void scheduler_block(tcb_t* tcb)
{
   disable_interrupts();
   LIST_REMOVE(running, tcb, scheduler_node);
   LIST_INSERT_BEFORE(blocked, tcb, scheduler_node);
   enable_interrupts();
}

tcb_t* scheduler_next()
{
   tcb_t* current = running;
   
   disable_interrupts();
   
   running = LIST_NEXT(running, scheduler_node);
   context_switch(&current->esp, running->esp);
   
   enable_interrupts();
   
   return running;
}

void scheduler_sleep(tcb_t* tcb, unsigned long ticks)
{
   //TODO
}






