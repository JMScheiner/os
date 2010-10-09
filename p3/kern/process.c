
#include <process.h>
#include <mm.h>
#include <assert.h>

static pid_t next_pid = 0;

DEFINE_HASHTABLE(pcb_table_t, pid_t, pcb_t *);

pcb_table_t pcb_table;

void init_pcb_table(void) {
	STATIC_INIT_HASHTABLE(pcb_table_t, pcb_table);
}

int initialize_process(pcb_t *pcb) {
	assert(pcb);
	pcb->pid = atomic_add(&next_pid, 1);
	pcb->thread_count = 0;
	pcb->page_directory = mm_new_directory();
	pcb->thread = NULL;
	mutex_init(&pcb->lock);
	return 0;
}
