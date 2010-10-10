
#include <thread.h>
#include <assert.h>

static int next_tid = 0;

DEFINE_HASHTABLE(tcb_table_t, int, tcb_t *);

tcb_table_t tcb_table;
tcb_table_t stack_table;

void thread_table_init(void) {
	STATIC_INIT_HASHTABLE(tcb_table_t, tcb_table);
	STATIC_INIT_HASHTABLE(tcb_table_t, stack_table);
}

void *initialize_thread(pcb_t *pcb, tcb_t *tcb) {
	assert(pcb);
	assert(tcb);
	tcb->tid = atomic_add(&next_tid, 1);
	tcb->pid = pcb->pid;
	mutex_lock(&pcb->lock);
	/* The thread list is a circular linked list. */
	if (pcb->thread) {
		tcb->next = pcb->thread->next;
		pcb->thread->next = tcb;
	}
	else {
		tcb->next = tcb;
		pcb->thread = tcb;
	}
	mutex_unlock(&pcb->lock);

	return allocate_kernel_stack(tcb);
}

#define STACK_TABLE_KEY(addr) ((unsigned int)(addr) / PAGE_SIZE)

void *allocate_kernel_stack(tcb_t *tcb) {
	void *stack = mm_new_kernel_page();
	mutex_lock(&stack_table_lock);
	HASHTABLE_PUT(stack_table, STACK_TABLE_KEY(stack), tcb);
	mutex_unlock(&stack_table_lock);
	tcb->esp = (char *)stack + PAGE_SIZE - WORD_SIZE; 
	return stack;
}

tcb_t get_tcb() {
	void *esp = get_esp();
	tcb_t *tcb;
	mutex_lock(&stack_table_lock);
	HASHTABLE_GET(stack_table, STACK_TABLE_KEY(esp), tcb);
	mutex_unlock(&stack_table_lock);
	return tcb;
}

void thr_start(tcb_t *tcb, regstate_t registers) {
	mode_switch(tcb->ret_addr);
}

