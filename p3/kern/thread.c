
#include <thread.h>
#include <assert.h>
#include <asm.h>
#include <hashtable.h>
#include <malloc.h>
#include <atomic.h>
#include <page.h>
#include <mm.h>
#include <asm_helper.h>
#include <reg.h>
#include <list.h>
#include <context_switch.h>
#include <scheduler.h>
#include <mutex.h>

static mutex_t tcb_table_lock;

/** @brief Number of pages per kernel stack. */

static int next_tid = 0xdeadbeef;

DEFINE_HASHTABLE(tcb_table_t, int, tcb_t *);

/* @brief Maps tids to tcbs.  */
tcb_table_t tcb_table;

/** 
* @brief Return a unique tid. 
* 
* @return A new tid. 
*/
int new_tid()
{
	return atomic_add(&next_tid, 1);
}

void init_thread_table(void) 
{
   mutex_init(&tcb_table_lock);
	STATIC_INIT_HASHTABLE(tcb_table_t, tcb_table, default_hash);
}

tcb_t* initialize_thread(pcb_t *pcb) 
{
	assert(pcb);
   
   void* kstack_page = mm_new_kernel_page();

   /* Put the TCB at the bottom of the kernel stack. */
   tcb_t* tcb = (tcb_t*)kstack_page;
	tcb->esp = kstack_page + PAGE_SIZE; 
   tcb->kstack = kstack_page + PAGE_SIZE;
	
   tcb->tid = new_tid();
	tcb->pcb = pcb;

   mutex_lock(&tcb_table_lock);
	HASHTABLE_PUT(tcb_table_t, tcb_table, tcb->tid, tcb);
	mutex_unlock(&tcb_table_lock);

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
	return (tcb_t *)PAGE_OF(esp);
}

void gettid_handler(volatile regstate_t reg)
{
   reg.eax = get_tcb()->tid;
   //MAGIC_BREAK;
}

/** 
* @brief Very similar functionality to initialize_thread, 
* 
* @return 
*/
tcb_t* kern_threadfork()
{
   pcb_t* pcb = get_pcb();
   
   tcb_t* current_tcb = get_tcb();
   tcb_t* new_tcb = initialize_thread(pcb);
   
   duplicate_thread_context(current_tcb->kstack, new_tcb->kstack, &new_tcb->esp);
   
   /* After registering, the new thread can be context switched to. */
   scheduler_register(new_tcb);
   return new_tcb;
}










