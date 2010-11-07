#include <kernel_types.h>
#include <global_thread.h>
#include <malloc.h>
#include <page.h>
#include <lifecycle.h>
#include <mutex.h>
#include <mm.h>

static pcb_t _global_pcb;
static tcb_t* _global_tcb;
static mutex_t _global_list_lock;

void global_thread_init()
{
   void* kstack;
   /* Give the "global process" a pcb and tcb. */
   _global_pcb.pid = -1;
   _global_pcb.parent = NULL;
   _global_pcb.thread_count = 1;
   _global_pcb.regions = NULL;
	mutex_init(&_global_pcb.directory_lock);
	mutex_init(&_global_pcb.region_lock);
	mutex_init(&_global_pcb.status_lock);
	mutex_init(&_global_pcb.waiter_lock);
	mutex_init(&_global_pcb.check_waiter_lock);
   _global_pcb.sanity_constant = PCB_SANITY_CONSTANT;

   LIST_INIT_NONEMPTY(&_global_pcb, global_node);
   mutex_init(&_global_list_lock);
   
   kstack = mm_new_kp_page() + PAGE_SIZE;
   _global_tcb = (tcb_t*)(kstack - PAGE_SIZE);
   _global_tcb->kstack = kstack;
   _global_tcb->esp = _global_tcb->kstack;
   _global_tcb->pcb = &_global_pcb;
   _global_tcb->tid = -1;
   _global_tcb->sanity_constant = TCB_SANITY_CONSTANT;
   _global_tcb->dir_p = _global_pcb.dir_p;

   arrange_global_context();
}

inline pcb_t* global_pcb() { 
   return &_global_pcb; 
} 

inline tcb_t* global_tcb() { 
   return _global_tcb; 
}

inline mutex_t* global_list_lock()
{
   return &_global_list_lock;
}

void global_list_remove(pcb_t* pcb)
{
   pcb_t* global = &_global_pcb;
   
   mutex_lock(&_global_list_lock);
   LIST_REMOVE(global, pcb, global_node); 
   mutex_unlock(&_global_list_lock);
}

void global_list_add(pcb_t* pcb)
{
   pcb_t* global = &_global_pcb;
   
   mutex_lock(&_global_list_lock);
   LIST_INSERT_AFTER(global, pcb, global_node); 
   mutex_unlock(&_global_list_lock);
}



