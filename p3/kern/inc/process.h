
#ifndef PROCESS_HJSD67S
#define PROCESS_HJSD67S

typedef struct process_control_block pcb_t;

#include <thread.h>
#include <elf_410.h>

/** @brief Process control block structure. */
struct process_control_block {

	/** @brief Process id. */
	int pid;

	/** @brief Process id of our parent. */
	int ppid;

	/** @brief Number of kernel threads running within the process. */
	int thread_count;

	/** @brief Base address of the process page directory. */
	void *page_directory;

	/** @brief Pointer to the next thread in this process that will execute. */
	tcb_t *thread;

	/** @brief Mutual exclusion lock for pcb fields. */
	//mutex_t lock;

};

void init_process_table(void);
int initialize_process(pcb_t *pcb);
int get_pid(void);
int initialize_memory(const char *file, simple_elf_t elf);

#endif

