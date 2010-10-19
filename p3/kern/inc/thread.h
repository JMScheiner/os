
#ifndef THREAD_FWFWJ23E
#define THREAD_FWFWJ23E

#define KERNEL_STACK_SIZE 1

#include <thread_type.h>
#include <process.h>
#include <list.h>

DEFINE_LIST(tcb_node_t, tcb_t);

/** @brief Thread control block structure. */
struct thread_control_block {

	/** @brief Thread id. */
	int tid;

	/** @brief Process id. */
	int pid;

	/** @brief Saved kernel stack pointer for this thread. */
	void *esp;
	
   /** @brief Bottom of the kernel stack. */
	void *kstack;

	/** @brief Next thread in the same process. */
	struct thread_control_block *next;

   tcb_node_t scheduler_node;
   tcb_node_t mutex_node;
   int runnable;

   unsigned long sleep_until;
   
};

int new_tid(void);
void init_thread_table(void);
tcb_t* initialize_thread(pcb_t *pcb);
void *allocate_kernel_stack(tcb_t *tcb);
tcb_t *get_tcb(void);

#endif

