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
#include <process.h>
#include <list.h>
#include <x86/asm.h>

static pcb_t* running;
static pcb_t* blocked;

void scheduler_init()
{
   INIT_LIST(running);   
   INIT_LIST(blocked); 
}

void scheduler_register(pcb_t* pcb)
{
   LIST_INIT_NODE(pcb, scheduler_node);   
   
   disable_interrupts();
   LIST_INSERT_AFTER(running, pcb, scheduler_node);
   enable_interrupts();
}

void scheduler_block(pcb_t* pcb)
{
   disable_interrupts();
   LIST_REMOVE(running, pcb, scheduler_node);
   LIST_INSERT_BEFORE(blocked, pcb, scheduler_node);
   enable_interrupts();
}

pcb_t* scheduler_next()
{
   disable_interrupts();
   running = LIST_NEXT(running, scheduler_node);
   enable_interrupts();
   return running;
}

void scheduler_sleep(pcb_t* pcb, unsigned long ticks)
{
   //TODO
}



