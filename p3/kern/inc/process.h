
#ifndef PROCESS_HJSD67S
#define PROCESS_HJSD67S

/** @brief Process identifier type. */
typedef unsigned int pid_t;

/** @brief Process control block structure. */
typedef struct {

	/** @brief Process id. */
	pid_t pid;

	/** @brief Number of kernel threads running within the process. */
	int thread_count;

	/** @brief Base address of the process page directory. */
	void *page_directory;

	/** @brief Pointer to the next thread in this process that will execute. */
	tcb_t *thread;

	/** @brief Mutual exclusion lock for pcb fields. */
	mutex_t lock;

} pcb_t;

void init_pcb_table(void);
int initialize_process(pcb_t *pcb);

#endif

