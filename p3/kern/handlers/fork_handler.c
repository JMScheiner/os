
#include <reg.h>
#include <process.h>
#include <thread.h>
#include <scheduler.h>
#include <context_switch.h>
#include <atomic.h>
#include <simics.h>
#include <mm.h>
#include <asm_helper.h>
#include <string.h>
#include <handlers/handler_wrappers.h>

/* The offset of a POPA in our assembly wrappers. */
#define POPA_OFFSET 6 

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
  
   /* Duplicate the current address space in the new process. */
   mm_duplicate_address_space(new_pcb);
   
   /* Arrange the new processes context for it's first context switch. */

   /* First give it a proper "iret frame" */
   new_tcb->esp = new_tcb->kstack;
   new_tcb->esp -= sizeof(regstate_t);
   memcpy(new_tcb->esp, (void*)&reg, sizeof(regstate_t));
   
   /* Push the return address for context switches ret */
   new_tcb->esp -= 4; 
   void (**ret_site)(void);
   ret_site = new_tcb->esp;

   /* asm_fork_handler + 8 will hopefully be the popa instruction. */
   (*ret_site) = (asm_fork_handler + POPA_OFFSET);
   
   /* Set up the context context_switch will popa off the stack. */
   new_tcb->esp -= sizeof(pusha_t);
   pusha_t* pusha = (pusha_t*)new_tcb->esp;
   pusha->eax = (unsigned long)new_pcb->page_directory;
   
   /* Register the first thread in the new TCB. */
   scheduler_register(new_tcb);
   reg.eax = newpid;
   return /* ! */;
}


