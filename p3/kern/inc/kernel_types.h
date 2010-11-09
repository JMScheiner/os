/** @file kernel_types.h
 *
 * @brief Data structures used throughout the kernel
 *
 * @author Justin Scheiner
 * @author Tim Wilson
 */

#ifndef KERNEL_TYPES_7FFQEKPQ
#define KERNEL_TYPES_7FFQEKPQ

#include <list.h>
#include <types.h>

/** @brief Arbitrary magic constants to identify corruption of our data
 * structures. */
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
typedef struct HASHTABLE_LINK hashtable_link_t;
typedef struct HASHTABLE hashtable_t;

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
	void (*fault)(void* addr, int ecode);

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
	mutex_t region_lock, directory_lock, status_lock, 
					waiter_lock, check_waiter_lock, child_lock;
   
   /** @brief Our node in a global list of PCBs, used when allocating new 
    *  tables for kernel virtual memory. */
   pcb_node_t global_node;

	/** @brief Circular list of our children. */
	pcb_t *children;

	/** @brief Our node in our parents children list. */
	pcb_node_t child_node;

	/** @brief Signal to indicate a child process has vanished. */
	cond_t wait_signal;

	/** @brief A magic constant that should not be changed. If it changes,
	 * memory has been corrupted. */
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
   
   /** @brief A pointer to our current directory, for context switching. */
   void *dir_p; 
	
   /** @brief Bottom of the kernel stack. */
	void *kstack;

	/** @brief Pointer to our place in either the scheduled list or
	 * descheduled list. If we are blocked we will be in neither list. */
   tcb_node_t scheduler_node;

	/** @brief True iff we are currently blocked. */
	boolean_t blocked;

	/** @brief True iff we are currently descheduled. */
	boolean_t descheduled;

	/** @brief Mutual exclusion lock to prevent us from interleaving
	 * deschedules with make_runnables. */
	mutex_t deschedule_lock;

	/** @brief If non-zero, we are sleeping and this is the time we should
	 * be woken up at. */
   unsigned long wakeup;

	/** @brief Our position in the sleep heap. */
   int sleep_index;

	/** @brief A magic constant that should not be changed. If it changes,
	 * the kernel stacks have probably been overflowed. */
   int sanity_constant;
};

struct SLEEP_HEAP 
{
   /** @brief Index of the first empty slot in the heap (size of heap + 1). */
   int index;

	/** @brief Number of allocated entries in the heap. */
   int size; 

	/** @brief Table of elements in the heap. */
   tcb_t** data; 
};

struct HASHTABLE_LINK
{
	/** @brief ID of the thread in this link. */
	int tid;

	/** @brief tcb of the thread in this link. */
	tcb_t *tcb;

	/** @brief Next link in the same table slot. */
	struct HASHTABLE_LINK *next;
};

struct HASHTABLE
{
	/** @brief Number of elements in the table. */
	size_t size;

	/** @brief Index into prime_table_sizes indicating the number of slots
	 * in the table. */
	size_t table_index;

	/** @brief Hash function mapping keys to table slots. */
	unsigned int (*hash)(int);

	/** @brief Mutual exclusion lock to protect the data. */
	mutex_t lock;

	/** @brief The hashtable. */
	hashtable_link_t **table;
};

#endif /* end of include guard: KERNEL_TYPES_7FFQEKPQ */


