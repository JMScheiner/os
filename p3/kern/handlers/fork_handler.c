
#include <reg.h>
#include <process.h>
#include <thread.h>
#include <scheduler.h>
#include <context_switch.h>

void thread_fork_handler(volatile regstate_t reg)
{
   unsigned long new_tid;
   pcb_t* pcb;
   tcb_t* current_tcb, *new_tcb;

   pcb = get_pcb();
   current_tcb = get_tcb();
   
   new_tcb = initialize_thread(pcb);
   new_tid = new_tcb->tid;
   
   /* This function should "return" twice */
   duplicate_thread_context(current_tcb->kstack, new_tcb->kstack, &new_tcb->esp);
   
   /* The following code executes twice... */
   current_tcb = get_tcb();
   if(current_tcb->tid != new_tid)
   {
      /* After registering, the new thread can be context switched to. */
      scheduler_register(new_tcb);
      reg.eax = new_tid;
   }
   else reg.eax = 0;
      
}

void fork_handler(volatile regstate_t reg)
{
   //TODO
}


