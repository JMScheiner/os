#ifndef KERNEL_TYPES_7FFQEKPQ

#define KERNEL_TYPES_7FFQEKPQ

#include <list.h>
#include <types.h>

#define TCB_SANITY_CONSTANT 0xdeadbeef
#define PCB_SANITY_CONSTANT 0xcafebabe

typedef struct MUTEX_NODE mutex_node_t;
typedef struct MUTEX mutex_t;
typedef struct COND cond_t;
typedef struct REGION region_t;
typedef struct STATUS status_t;
typedef struct PROCESS_CONTROL_BLOCK pcb_t;
typedef struct THREAD_CONTROL_BLOCK tcb_t;
typedef struct SLEEP_HEAP sleep_heap_t;

DEFINE_LIST(tcb_node_t, tcb_t);
DEFINE_LIST(pcb_node_t, pcb_t);

/** @brief Queue node in a mutex. */
struct MUTEX_NODE {
	/** @brief tcb of the waiting thread. */
	tcb_t *tcb;

	/** @brief Next waiting thread. */
	struct MUTEX_NODE *next;
};

/** @brief Mutual exclusion lock with a waiting queue. */
struct MUTEX {
	/** @brief The first thread waiting on the mutex. */
	mutex_node_t *head;

	/** @brief The last thread waiting on the mutex. */
	mutex_node_t *tail;

	/** @brief True iff someone is holding the mutex. */ 
	boolean_t locked;

	/** @brief Simple check to protect against access before intialization
	 * or after destruction. */
	boolean_t initialized;
};

/** @brief Simple condition variable supporting up to one waiter at a 
 * time. */
struct COND {
	/** @brief True if this has been passed to cond_init. False if this 
	 * has been passed to cond_destroy. */
	boolean_t initialized;

	/** @brief The tcb of a thread waiting on this condition variable. */
	tcb_t *tcb;
};

/** @brief Struct representing a user region of memory. */
struct REGION
{
	/** @brief The first address of the region. */
	void* start;

	/** @brief The first address above the region. */
	void* end;

	/** @brief The page fault handler for the region. */
	void (*fault)(void* addr, int access_mode);

	/** @brief The next region in the address space. */
	struct REGION* next;
};

/** @brief Status block structure to store the exit status of a process. */
struct STATUS {
	/** @brief The exit status. Set by the set_status system call. */
	int status;

	/** @brief The tid of the original thread in the process. */
	int tid;

	/** @brief A next pointer to make a list of status blocks of exited 
	 * children. */
	struct STATUS *next;
};

/** @brief Process control block structure. */
struct PROCESS_CONTROL_BLOCK
{
	/** @brief Process id. */
	int pid;

	/** @brief pcb of our parent. */
	pcb_t *parent;

	/** @brief Circular list of our children. */
	pcb_t *children;

	/** @brief Number of kernel threads running within the process. */
	int thread_count;

	/** @brief Number of child processes, either alive or zombies, minus
	 * the number of waiting parent threads. */
	int unclaimed_children;

	/** @brief A list of regions with different page fault and freeing 
	 * procedures. */
	region_t *regions;
	
	/** @brief Our exit status. */
	status_t *status;

	/** @brief Pointer to the list of exited child statuses. */
	status_t *zombie_statuses;

	/** @brief Base phys and virt addresses of the processes page directory. */
	void *dir_p;
	void *dir_v;

	/** @brief Translates addresses to virtual table addresses*/
	void *virtual_dir;
	
	/** @brief Mutual exclusion locks for pcb. */
	mutex_t lock, region_lock, directory_lock, status_lock, 
					waiter_lock, check_waiter_lock, child_lock, kvm_lock;
   
   pcb_node_t global_node;
	pcb_node_t child_node;

	/** @brief Signal to indicate a child process has vanished. */
	cond_t wait_signal;
   int sanity_constant;
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

   unsigned long wakeup;
   int sleep_index;
   int sanity_constant;
};

struct SLEEP_HEAP 
{
   /* Refers to the first free slot. */
   int index;
   int size; 
   tcb_t** data; 
};

#endif /* end of include guard: KERNEL_TYPES_7FFQEKPQ */


