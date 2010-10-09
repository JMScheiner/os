
#ifndef THREAD_FWFWJ23E
#define THREAD_FWFWJ23E

/** @brief Thread identifier type. */
typedef unsigned int tid_t;

/** @brief Thread control block structure. */
typedef struct {

	/** @brief Thread id. */
	tid_t tid;

	/** @brief Process id. */
	pid_t pid;

	/** @brief Address to return to after a kernel to user mode switch. */
	void *ret_addr

	/** @brief Next thread in the same process. */
	tcb_t *next;

} tcb_t;

int initialize_thread(pcb_t *pcb, tcb_t *tcb);

#endif

