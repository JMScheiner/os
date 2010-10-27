#ifndef KERNEL_TYPES_7FFQEKPQ

#define KERNEL_TYPES_7FFQEKPQ

#include <list.h>
#include <types.h>

typedef struct MUTEX_NODE mutex_node_t;
typedef struct MUTEX mutex_t;
typedef struct REGION region_t;
typedef struct PROCESS_CONTROL_BLOCK pcb_t;
typedef struct THREAD_CONTROL_BLOCK tcb_t;
typedef struct COND cond_t;

DEFINE_LIST(tcb_node_t, tcb_t);

struct MUTEX_NODE {
	tcb_t *tcb;
	struct MUTEX_NODE *next;
};

struct MUTEX{
	mutex_node_t *head;
	mutex_node_t *tail;
	boolean_t locked;
	boolean_t initialized;
};

struct REGION
{
   void* start;
   void* end;

   void (*fault)(void* addr, int access_mode);
   struct REGION* next;
};

/** @brief Process control block structure. */
struct PROCESS_CONTROL_BLOCK
{
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
	
	/** @brief Pointer to the list of exited child statuses. */
	status_t *zombie_statuses;

	/** @brief Exit status of this process. */
	status_t status;

	/** @brief Mutual exclusion lock for pcb fields. */
	mutex_t lock, region_lock, directory_lock, status_lock;
};

/** @brief Thread control block structure. */
struct THREAD_CONTROL_BLOCK{

	/** @brief Thread id. */
	int tid;

	/** @brief Process control block. */
	pcb_t *pcb;

	/** @brief Saved kernel stack pointer for this thread. */
	void *esp;
	
   /** @brief Bottom of the kernel stack. */
	void *kstack;

	/** @brief Next thread in the same process. */
	struct THREAD_CONTROL_BLOCK *next;

   tcb_node_t scheduler_node;
   tcb_node_t mutex_node;
   int runnable;

   unsigned long sleep_until;
};

struct COND {
	/** @brief True if this has been passed to cond_init. False if this has been
	 * passed to cond_destroy. */
	boolean_t initialized;

	/** @brief The tcb of a thread waiting on this condition variable. */
	tcb_t *tcb;
};

#endif /* end of include guard: KERNEL_TYPES_7FFQEKPQ */


