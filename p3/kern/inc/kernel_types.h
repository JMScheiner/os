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
typedef struct SLEEP_HEAP sleep_heap_t;

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

	/** @brief Base phys and virt addresses of the processes page directory. */
	void *dir_p;
	void *dir_v;
   
   /** @brief Translates addresses to virtual table addresses*/
   void *virtual_dir;
   
   /** @brief A list of regions with different page fault procedures. */
   region_t* regions;

	/** @brief Mutual exclusion lock for pcb fields. */
	mutex_t lock, region_lock, directory_lock, kvm_lock;
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
};

struct COND {
	mutex_t lock;
};

struct SLEEP_HEAP 
{
   /* Refers to the first free slot. */
   int index;
   int size; 
   tcb_t** data; 
};

#endif /* end of include guard: KERNEL_TYPES_7FFQEKPQ */


