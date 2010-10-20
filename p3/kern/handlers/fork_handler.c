
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
#include <cr.h>
#include <pop_stub.h>

void* arrange_fork_context(void* esp, regstate_t* reg, void* page_directory);

void thread_fork_handler(volatile regstate_t reg)
{
   unsigned long newtid;
   pcb_t* pcb;
   tcb_t* current_tcb, *new_tcb;

   pcb = get_pcb();
   current_tcb = get_tcb();
   
   new_tcb = initialize_thread(pcb);
   newtid = new_tcb->tid;
   atomic_add(&pcb->thread_count, 1);
   
   //lprintf("new_tcb->kstack = %p", new_tcb->kstack);
   new_tcb->esp = arrange_fork_context(
      new_tcb->kstack, (regstate_t*)&reg, (void*)get_cr3());
   
   //lprintf("Registering task 0x%x with esp = %p", new_tcb->tid, new_tcb->esp);

   scheduler_register(new_tcb);
   reg.eax = newtid;
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
   new_tcb->esp = arrange_fork_context(
      new_tcb->kstack, (regstate_t*)&reg, new_pcb->page_directory);
   
   /* Register the first thread in the new TCB. */
   scheduler_register(new_tcb);
   
   reg.eax = newpid;
   return /* ! */;
}

void* arrange_fork_context(void* esp, regstate_t* reg, void* page_directory)
{
   /* First give it a proper "iret frame" */
   esp -= sizeof(regstate_t);
   memcpy(esp, (void*)reg, sizeof(regstate_t));
   
   /* Set eax to zero for the iret from either thread_fork or fork. */
   regstate_t* new_reg = (regstate_t*)esp;
   new_reg->eax = 0;
   
   /* Push the return address for context switches ret */
   esp -= 4; 
   void (**ret_site)(void);
   ret_site = esp;

   (*ret_site) = (pop_stub);
   
   /* Set up the context context_switch will popa off the stack. */
   esp -= sizeof(pusha_t);
   pusha_t* pusha = (pusha_t*)esp;
   pusha->eax = (unsigned long)page_directory;
   return esp;
}


