
#ifndef PROCESS_HJSD67S
#define PROCESS_HJSD67S

typedef struct process_control_block pcb_t;

#include <region.h>
#include <mutex.h>
#include <list.h>
#include <elf_410.h>

#define USER_STACK_BASE 0xc0000000

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

   /** @brief A list of regions with different page fault and freeing procedures. */
   region_t* regions;

	/** @brief Mutual exclusion lock for pcb fields. */
	mutex_t lock;

   /** @brief Lock for page directory. */
   mutex_t mm_lock;

};

void init_process_table(void);
int get_pid(void);
int initialize_memory(const char *file, simple_elf_t elf, pcb_t* pcb);
pcb_t* initialize_process();
pcb_t* initialize_first_process();
pcb_t* get_pcb(void);

#endif

