
#include <thread.h>
#include <assert.h>

static tid_t next_tid = 0;

int initialize_thread(pcb_t *pcb, tcb_t *tcb) {
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
}

void thr_start(tcb_t *tcb, regstate_t registers) {
	mode_switch(tcb->ret_addr);
}

