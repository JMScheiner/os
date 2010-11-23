/** @file global.c
 *
 * @brief Functions to initialize and fetch the global pcb and tcb (our
 * version of idle) that should be switched to when there is no one else
 * to run. Also functions to add and remove ourself from the
 * global list of all pcbs.
 *
 * @author Tim Wilson
 * @author Justin Scheiner
 */

#include <kernel_types.h>
#include <global_thread.h>
#include <malloc.h>
#include <page.h>
#include <lifecycle.h>
#include <mutex.h>
#include <cond.h>
#include <mm.h>
#include <types.h>

static pcb_t _global_pcb;
static tcb_t* _global_tcb;
static mutex_t _global_list_lock;

/** 
* @brief Initializes a global PCB that generally stands in
*  when no true PCB is appropriate. It is spiritually the PCB 
*  of the idle kernel process. 
*/
void global_thread_init()
{
   void* kstack;
   /* Give the "global process" a pcb and tcb. */
   _global_pcb.pid = -1;
   _global_pcb.parent = NULL;
   _global_pcb.thread_count = 1;
   _global_pcb.unclaimed_children = 0;
   _global_pcb.vanishing_children = 0;
   _global_pcb.vanishing = FALSE;
   _global_pcb.regions = NULL;
   mutex_init(&_global_pcb.directory_lock);
   mutex_init(&_global_pcb.region_lock);
   mutex_init(&_global_pcb.status_lock);
   mutex_init(&_global_pcb.waiter_lock);
   mutex_init(&_global_pcb.check_waiter_lock);
   mutex_init(&_global_pcb.child_lock);
   mutex_init(&_global_pcb.swexn_lock);
   
   cond_init(&_global_pcb.wait_signal);
   cond_init(&_global_pcb.vanish_signal);

   _global_pcb.sanity_constant = PCB_SANITY_CONSTANT;

   LIST_INIT_NONEMPTY(&_global_pcb, global_node);
   mutex_init(&_global_list_lock);
   
   kstack = mm_new_kp_page() + PAGE_SIZE;
   assert(kstack - PAGE_SIZE != NULL);

   _global_tcb = (tcb_t*)(kstack - PAGE_SIZE);
   _global_tcb->kstack = kstack;
   _global_tcb->esp = _global_tcb->kstack;
   _global_tcb->pcb = &_global_pcb;
   _global_tcb->tid = -1;
   _global_tcb->wakeup = 0;
   _global_tcb->sleep_index = 0;
   _global_tcb->sanity_constant = TCB_SANITY_CONSTANT;
   _global_tcb->dir_p = _global_pcb.dir_p;
   cond_init(&_global_tcb->swexn_signal);

   arrange_global_context();
}

/** 
* @brief Return the global PCB. 
* 
* @return The global PCB.
*/
inline pcb_t* global_pcb() { 
   return &_global_pcb; 
} 

/** 
* @brief Return the global tcb. 
* 
* @return The global TCB.
*/
inline tcb_t* global_tcb() { 
   return _global_tcb; 
}

/** 
* @brief Return the global list lock - e.g. 
*   the lock for the list in each PCB's global_node.
* 
* @return The lock.
*/
inline mutex_t* global_list_lock()
{
   return &_global_list_lock;
}

/** 
* @brief Remove the PCB from the global list. 
* 
* @param pcb The PCB to remove. 
*/
void global_list_remove(pcb_t* pcb)
{
   pcb_t* global = &_global_pcb;
   
   mutex_lock(&_global_list_lock);
   LIST_REMOVE(global, pcb, global_node); 
   mutex_unlock(&_global_list_lock);
}

/** 
* @brief Add a PCB to the global list. 
* 
* @param pcb The PCB to add.
*/
void global_list_add(pcb_t* pcb)
{
   pcb_t* global = &_global_pcb;
   
   mutex_lock(&_global_list_lock);
   LIST_INSERT_AFTER(global, pcb, global_node); 
   mutex_unlock(&_global_list_lock);
}



