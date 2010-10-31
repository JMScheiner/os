#include <kernel_types.h>
#include <global_thread.h>
#include <malloc.h>
#include <page.h>
#include <lifecycle.h>

static pcb_t _global_pcb;
static tcb_t* _global_tcb;

void global_thread_init()
{
   void* kstack;
   /* Give the "global process" a pcb and tcb. */
   _global_pcb.pid = -1;
   _global_pcb.parent = NULL;
   _global_pcb.thread_count = 1;
   _global_pcb.regions = NULL;
   
   kstack = smemalign(PAGE_SIZE, PAGE_SIZE) + PAGE_SIZE;
   _global_tcb = (tcb_t*)(kstack - PAGE_SIZE);
   _global_tcb->kstack = kstack;
   _global_tcb->esp = _global_tcb->kstack;
   _global_tcb->pcb = &_global_pcb;
   _global_tcb->tid = -1;
   arrange_global_context();
}

inline pcb_t* global_pcb() { 
   return &_global_pcb; 
} 

inline tcb_t* global_tcb() { 
   return _global_tcb; 
} 

