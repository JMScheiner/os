
#ifndef THREAD_FWFWJ23E
#define THREAD_FWFWJ23E

typedef struct thread_control_block tcb_t;

#include <process.h>
#include <list.h>

DEFINE_LIST(tcb_node_t, tcb_t);

/** @brief Thread control block structure. */
struct thread_control_block {

	/** @brief Thread id. */
	int tid;

	/** @brief Process id. */
	int pid;

	/** @brief Top address of the kernel stack for this thread. */
	void *esp;

	/** @brief Next thread in the same process. */
	struct thread_control_block *next;

   tcb_node_t scheduler_node;
   tcb_node_t mutex_node;

   unsigned long sleep_until;
   
};

void init_thread_table(void);
void *initialize_thread(pcb_t *pcb, tcb_t *tcb);
void *allocate_kernel_stack(tcb_t *tcb);
tcb_t *get_tcb(void);

#endif

