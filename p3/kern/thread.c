
#include <thread.h>
#include <assert.h>
#include <asm.h>
#include <hashtable.h>
#include <malloc.h>
#include <atomic.h>
#include <page.h>
#include <mm.h>
#include <kvm.h>
#include <asm_helper.h>
#include <reg.h>
#include <list.h>
#include <context_switch.h>
#include <scheduler.h>
#include <mutex.h>
#include <debug.h>
#include <global_thread.h>

//static mutex_t tcb_table_lock;

/** @brief Number of pages per kernel stack. */

static int next_tid = 1;

hashtable_t tcb_table;

//DEFINE_HASHTABLE(tcb_table_t, int, tcb_t *);

/* @brief Maps tids to tcbs.  */
//tcb_table_t tcb_table;

/** 
* @brief Return a unique tid. 
* 
* @return A new tid. 
*/
static int new_tid()
{
	return atomic_add(&next_tid, 1);
}

void thread_init(void) 
{
	hashtable_init(&tcb_table, default_hash);
}

void free_thread_resources(tcb_t* tcb)
{
   kvm_free_page((void*)tcb);
}

tcb_t* initialize_thread(pcb_t *pcb) 
{
	assert(pcb);
   
   if(kvm_request_frames(0, 1) < 0)
      return NULL;
   
	void* kstack_page = kvm_new_page();
   assert(kstack_page);
   assert(mm_getflags(pcb, kstack_page) & PTENT_PRESENT);
	
   debug_print("mm", "new kernel stack page at %p", kstack_page);

	/* Put the TCB at the bottom of the kernel stack. */
	tcb_t* tcb = (tcb_t*)kstack_page;
	tcb->esp = kstack_page + PAGE_SIZE; 
   
   /* Keep a copy of the physical directory, since we can context switch 
    *  without the PCB */
   tcb->dir_p = pcb->dir_p;
   assert(tcb->dir_p);
	
   tcb->kstack = kstack_page + PAGE_SIZE;
	
	tcb->tid = new_tid();
	tcb->pcb = pcb;
	tcb->wakeup = 0;
	tcb->sleep_index = 0;
	tcb->blocked = FALSE;
	tcb->descheduled = FALSE;
	mutex_init(&tcb->deschedule_lock);
   tcb->sanity_constant = TCB_SANITY_CONSTANT;
	
   int siblings = atomic_add(&pcb->thread_count, 1);
	if (siblings == 0) {
		pcb->status->tid = tcb->tid;
	}

	LIST_INIT_NODE(tcb, scheduler_node);

	mutex_lock(&tcb_table.lock);
	hashtable_put(&tcb_table, tcb->tid, tcb);
	mutex_unlock(&tcb_table.lock);

	return tcb;
}

/**
 * @brief Get the tcb of this thread
 *
 * NOTE: Relies on kernel stacks being one page.
 *
 * @return The tcb
 */
tcb_t *get_tcb()
{
	void *esp = get_esp();
   tcb_t* ret = NULL;

   if(esp < global_tcb()->kstack)
      ret = global_tcb();
   else
      ret = (tcb_t*)PAGE_OF(esp);

   assert(ret->sanity_constant == TCB_SANITY_CONSTANT);
   
   assert(ret->pcb != NULL);
   assert(ret->pcb->sanity_constant = PCB_SANITY_CONSTANT);
	return ret;
}

void check_invariants(boolean_t check_thread_count) {
	tcb_t *tcb = get_tcb();
	assert(tcb);
	pcb_t *pcb = tcb->pcb;
	assert(pcb);
	assert(pcb->parent || pcb == global_pcb());
	if (check_thread_count)
		assert(pcb->thread_count > 0);
	assert(pcb->unclaimed_children >= 0);
	assert(pcb->regions || pcb == global_pcb());
	assert(pcb->status || pcb == global_pcb());
	assert(pcb->dir_p);
	assert(pcb->dir_v);
	assert(pcb->virtual_dir);
	assert(pcb->region_lock.initialized == TRUE);
	assert(pcb->directory_lock.initialized == TRUE);
	assert(pcb->status_lock.initialized == TRUE);
	assert(pcb->vanish_lock.initialized == TRUE);
	assert(pcb->waiter_lock.initialized == TRUE);
	assert(pcb->check_waiter_lock.initialized == TRUE);
	assert(pcb->child_lock.initialized == TRUE);
	assert(pcb->wait_signal.initialized == TRUE);
	assert(pcb->sanity_constant == PCB_SANITY_CONSTANT);

	assert(tcb->dir_p == pcb->dir_p);
	assert(((unsigned int)tcb->kstack & PAGE_MASK) == 0);
	assert(tcb->blocked == FALSE);
	assert(tcb->descheduled == FALSE);
	assert(tcb->deschedule_lock.initialized || tcb == global_tcb());
	assert(tcb->wakeup == 0);
	assert(tcb->sleep_index == 0);
	assert(tcb->sanity_constant == TCB_SANITY_CONSTANT);
}
