
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

/** @brief Number of pages per kernel stack. */
#define KERNEL_STACK_SIZE 1

static int next_tid = 0xdeadbeef;

DEFINE_HASHTABLE(tcb_table_t, int, tcb_t *);

tcb_table_t tcb_table;
tcb_table_t stack_table;

void init_thread_table(void) {
	STATIC_INIT_HASHTABLE(tcb_table_t, tcb_table, default_hash);
	STATIC_INIT_HASHTABLE(tcb_table_t, stack_table, default_hash);
}

void *initialize_thread(pcb_t *pcb, tcb_t *tcb) {
	assert(pcb);
	assert(tcb);
	tcb->tid = atomic_add(&next_tid, 1);
	tcb->pid = pcb->pid;
	disable_interrupts();
	//mutex_lock(&pcb->lock);
	/* The thread list is a circular linked list. */
	if (pcb->thread) {
		tcb->next = pcb->thread->next;
		pcb->thread->next = tcb;
	}
	else {
		tcb->next = tcb;
		pcb->thread = tcb;
	}
	enable_interrupts();
	//mutex_unlock(&pcb->lock);

	return allocate_kernel_stack(tcb);
}

#define STACK_TABLE_KEY(addr) ((unsigned int)(addr) / PAGE_SIZE)

void *allocate_kernel_stack(tcb_t *tcb) {
	void *stack = mm_new_kernel_pages(KERNEL_STACK_SIZE);
	disable_interrupts();
	//mutex_lock(&stack_table_lock);
	HASHTABLE_PUT(tcb_table_t, stack_table, STACK_TABLE_KEY(stack), tcb);
	enable_interrupts();
	//mutex_unlock(&stack_table_lock);
	tcb->esp = (char *)stack + PAGE_SIZE; 
	return stack;
}

tcb_t *get_tcb() {
	void *esp = get_esp();
	tcb_t *tcb = NULL;
	disable_interrupts();
	//mutex_lock(&stack_table_lock);
	HASHTABLE_GET(tcb_table_t, stack_table, STACK_TABLE_KEY(esp), tcb);
	enable_interrupts();
	//mutex_unlock(&stack_table_lock);
	return tcb;
}

void gettid_handler(volatile regstate_t reg)
{
   reg.eax = get_tcb()->tid;
   //MAGIC_BREAK;
}




