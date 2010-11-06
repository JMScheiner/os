
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
int new_tid()
{
	return atomic_add(&next_tid, 1);
}

void thread_init(void) 
{
	hashtable_init(&tcb_table, default_hash);
}

tcb_t* initialize_thread(pcb_t *pcb) 
{
	assert(pcb);
	//assert(pcb != (void*)0xffffffff);
   
	void* kstack_page = kvm_new_page();
	debug_print("mm", "new kernel stack page at %p", kstack_page);

	/* Put the TCB at the bottom of the kernel stack. */
	tcb_t* tcb = (tcb_t*)kstack_page;
	
	tcb->esp = kstack_page + PAGE_SIZE; 
	tcb->kstack = kstack_page + PAGE_SIZE;
	
	tcb->tid = new_tid();
	tcb->pcb = pcb;
	tcb->wakeup = 0;
	tcb->blocked = FALSE;
	tcb->descheduled = FALSE;
   tcb->sanity_constant = TCB_SANITY_CONSTANT;
	int siblings = atomic_add(&pcb->thread_count, 1);
	if (siblings == 0) {
		pcb->status->tid = tcb->tid;
	}

	//LIST_INIT_NODE(tcb, scheduler_node);
	LIST_INIT_NODE(tcb, mutex_node);

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
   
   /* TODO When is this NULL? */
   assert(ret->pcb != NULL);
   assert(ret->pcb->sanity_constant = PCB_SANITY_CONSTANT);
	return ret;
}

