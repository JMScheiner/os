
#include <reg.h>
#include <process.h>
#include <thread.h>
#include <scheduler.h>
#include <context_switch.h>
#include <atomic.h>
#include <simics.h>
#include <mm.h>
#include <asm_helper.h>


void thread_fork_handler(volatile regstate_t reg)
{
   unsigned long new_tid;
   pcb_t* pcb;
   tcb_t* current_tcb, *new_tcb;

   pcb = get_pcb();
   current_tcb = get_tcb();
   
   new_tcb = initialize_thread(pcb);
   new_tid = new_tcb->tid;
   atomic_add(&pcb->thread_count, 1);
   
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
   unsigned long newpid; 
   pcb_t *current_pcb, *new_pcb; 
   tcb_t *current_tcb, *new_tcb;

   current_pcb = get_pcb();
   current_tcb = get_tcb();
   
   new_pcb = initialize_process();
   
   new_tcb = initialize_thread(new_pcb);
   new_pcb->thread_count = 1;
   newpid = new_pcb->pid;
   
   lprintf("Passing fresh page directory at %p", new_pcb->page_directory);
   mm_duplicate_address_space(new_pcb);
   
   MAGIC_BREAK;
   duplicate_proc_context(
      current_tcb->kstack, new_tcb->kstack, &new_tcb->esp, new_pcb->page_directory);
   
   lprintf("Back! Saved esp = %p, my esp is %p", 
      new_tcb->esp, get_esp());
      
   current_pcb = get_pcb();
   
   MAGIC_BREAK;
   if(current_pcb->pid != newpid)
   {
      scheduler_register(new_tcb);
      reg.eax = newpid;
   }
   else reg.eax = 0;
}


