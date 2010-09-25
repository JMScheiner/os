/** @file threads.c
 *
 * @brief Thread management API for user programs
 *
 * @author Tim Wilson
 */

#include <thread.h>

#include <mutex.h>
#include <cond.h>
#include <thr_internals.h>
#include <thread_helper.h>
#include <thread_fork.h>
#include <hashtable.h>
#include <syscall.h>
#include <stdio.h>
#include <types.h>
#include <assert.h>
#include <stdlib.h>
#include <simics.h>

/** @brief The alignment of the stack pointer. This must be a power of 2. */
#define ESP_ALIGN 4

/** @brief The size of the kill stack. Must be large enough to hold a call to
 * free. */
#define KILL_STACK_SIZE 1024

/** @brief The size of a child stack. Initialized in thr_init. */
static unsigned int stack_size;

/** @brief Has thr_init been called? */
static boolean_t initialized = FALSE;

/** @brief The logical top of the shared kill stack. */
static char kill_stack_top[KILL_STACK_SIZE + 2*ESP_ALIGN - 1];

/** @brief The base of the shared kill stack. A thread jumps to this stack just
 * before it exits so it can free its own stack. */
static char *kill_stack;

/** @brief A lock on the kill stack. */
static mutex_t kill_stack_lock;

/** @brief Define a hashtable type from int to tcb_t *. */
DEFINE_HASHTABLE(hashtable_t, int, tcb_t *);

/** @brief A table mapping tids to thread control blocks. The main parent
 * thread will be in this table. */
static hashtable_t tid_table;

/** @brief A lock for the tid_table. */
static mutex_t tid_table_lock;

/** @brief A table mapping stack addresses (kind of) to thread control blocks.
 * The main parent thread will not be in this table. */
static hashtable_t stack_table;

/** @brief A lock for the stack_table. */
static mutex_t stack_table_lock;

/** @brief The maximum stack address a child thread could have. */
static char *max_child_stack_addr = NULL;

/** @brief A lock to synchronize writes to the maximum stack address. */
static mutex_t max_child_stack_addr_lock;

/** @brief The thread control block for the main parent thread. */
static tcb_t main_thread;

/** @brief Hash an address to the index of the range the address lies in.
 *
 * This function has the property that prehash(addr) == prehash(stack_top) or
 * prehash(addr) == 1 + prehash(stack_top) if addr lies on the stack whose top
 * is stack_top.
 *
 * Furthermore, prehash(stack1) != prehash(stack2) if stack1 and stack2 are the
 * tops of different stacks.
 *
 * @param addr The address to hash.
 *
 * @return An index that identifies which stack addr is on.
 */
static unsigned int prehash(char *addr) {
	return ((unsigned int)addr) / (stack_size + 2*ESP_ALIGN - 1);
}

/** @brief Identity hash function
 *
 * @param key The key to hash
 *
 * @return The hash value
 */
static unsigned int hash(int key) {
	return (unsigned int)key;
}

/** @brief Initialize the thread library
 *
 * Also, create a thread control block for the main parent thread.
 *
 * @param size The amount of stack space in bytes that each thread will
 *        have available
 *
 * @return 0 on success, less than 0 on error
 */
int thr_init(unsigned int size) {
	assert(!initialized);
	int ret = 0;
	initialized = TRUE;
	stack_size = size;

	/* Initialize the main_thread block. */
	main_thread.stack = NULL;
	main_thread.tid = gettid();
	ret |= mutex_init(&(main_thread.lock));
	ret |= cond_init(&(main_thread.signal));

	main_thread.initialized = TRUE;
	main_thread.exited = FALSE;

	/* Initialize the hash tables and their locks. */
	STATIC_INIT_HASHTABLE(hashtable_t, tid_table, hash);
	STATIC_INIT_HASHTABLE(hashtable_t, stack_table, hash);
	HASHTABLE_PUT(hashtable_t, tid_table, main_thread.tid, &main_thread);
	ret |= mutex_init(&tid_table_lock);
	ret |= mutex_init(&stack_table_lock);

	/* Initialize the max stack address lock. */
	ret |= mutex_init(&max_child_stack_addr_lock);

	/* Initialize the kill stack and its lock. */
	kill_stack = kill_stack_top + KILL_STACK_SIZE + ESP_ALIGN - 1;
	kill_stack = (char *) (((unsigned int)kill_stack) & ~(ESP_ALIGN - 1));
	ret |= mutex_init(&kill_stack_lock);
	assert(ret == 0);
	return ret;
}

/** @brief Create a new thread running the given function
 *
 * @param func The function the new thread will begin running
 *
 * @param arg The sole argument to func
 *
 * @return The thread ID of the new thread on success, less than 0 on error
 */
int thr_create(void *(*func)(void *), void *arg) {
	assert(initialized);
	int ret = -1;

	/* Create a thread control block and initialize its stack, mutex, and
	 * condition variable. */
	tcb_t *tcb = (tcb_t *)malloc(sizeof(tcb_t));
	assert(tcb);
	tcb->exited = FALSE;
	tcb->initialized = FALSE;
	if(mutex_init(&tcb->lock)) {
		goto fail_mutex;
	}
	if (cond_init(&tcb->signal)) {
		goto fail_cond;
	}
	tcb->stack = (char *)malloc(stack_size + 2*ESP_ALIGN - 1);
	assert(tcb->stack);
	
	/* Compute the base address of the child stack. */
	char *stack_bottom = tcb->stack + stack_size + ESP_ALIGN - 1;
	stack_bottom = (char *) (((unsigned int)stack_bottom) & ~(ESP_ALIGN - 1));
	
	/* Update the max child stack address. */
	assert(mutex_lock(&max_child_stack_addr_lock) == 0);
	if (max_child_stack_addr < stack_bottom) {
		max_child_stack_addr = stack_bottom;
	}
	assert(mutex_unlock(&max_child_stack_addr_lock) == 0);
	
	/* Fork the new child thread. If we fail, undo all the initialization we've
	 * done. */
	ret = thread_fork(func, arg, stack_bottom, tcb);
	if (ret >= 0) {
		return ret;
	}

	assert(cond_destroy(&tcb->signal) == 0);
fail_cond:
	assert(mutex_destroy(&tcb->lock) == 0);
fail_mutex:
	free(tcb->stack);
	free(tcb);
	return ret;
}

/** @brief Initialize a new child thread. 
 *
 * The tid still needs to be fetched, and the child needs to add its tcb to the
 * hashtables.
 *
 * @param tcb The partial thread control block of the thread. 
 */
void thr_child_init(void *(*func)(void*), void* arg, tcb_t* tcb) {
	assert(tcb);
	
	lprintf("In thr_child_init at address %p, tcb = %p. \n",  
		get_addr(), tcb);
	tcb->tid = gettid();
	lprintf("[%d] Finished getting tid.\n", tcb->tid);
	
	assert(mutex_lock(&tid_table_lock) == 0);
	HASHTABLE_PUT(hashtable_t, tid_table, tcb->tid, tcb);
	assert(mutex_unlock(&tid_table_lock) == 0);
	
	unsigned int key = prehash(tcb->stack);
	
	assert(mutex_lock(&stack_table_lock) == 0);
	HASHTABLE_PUT(hashtable_t, stack_table, key, tcb);
	assert(mutex_unlock(&stack_table_lock) == 0);

	/* Notify the parent that we've fully initialized ourself. */
	assert(mutex_lock(&tcb->lock) == 0);
	tcb->initialized = TRUE;
	assert(mutex_unlock(&tcb->lock) == 0);
	assert(cond_signal(&tcb->signal) == 0);
	thr_exit(func(arg));
}

/** @brief Wait for our child to completely initialize itself.
 *
 * We must wait for our child to move to its new stack before we continue.
 * Otherwise we might clobber parts of our stack before our child leaves it.
 * Additionally we must wait for our child to set its tid in its tcb and add
 * itself to the two tcb tables.
 *
 * @param tcb The tcb of the child we're waiting on.
 */
void wait_for_child(tcb_t *tcb) {
	assert(tcb);
	assert(mutex_lock(&tcb->lock) == 0);

	lprintf("[%d] Waiting for child\n", thr_getid());
	while (!tcb->initialized) {
		assert(cond_wait(&tcb->signal, &tcb->lock) == 0);
	}
	assert(mutex_unlock(&tcb->lock) == 0);
}	

/** @brief Wait for the specified thread to exit and collect its status
 *
 * @param tid The ID of the thread to wait for. It is an error to pass a tid
 *        that has not yet been created by thr_create
 *
 * @param statusp If not NULL, the status returned by the exiting thread will
 *        be placed here
 *
 * @return 0 on success, less than 0 on error
 */
int thr_join(int tid, void **statusp) {
	assert(initialized);
	tcb_t *tcb = NULL; // = &tcb_struct;

	MAGIC_BREAK;
	/* Get the tcb corresponding to this tid from the tid_table. */
	assert(mutex_lock(&tid_table_lock) == 0);
	HASHTABLE_GET(hashtable_t, tid_table, tid, tcb);

	/* If we're not the first to join on this thread, exit promptly. */
	if (tcb) {
		/* Remove the joined thread from the tid_table. */
		HASHTABLE_REMOVE(hashtable_t, tid_table, tid, tcb);
		assert(mutex_unlock(&tid_table_lock) == 0);

		/* Wait for the joined thread to signal completion. */
		assert(mutex_lock(&tcb->lock) == 0);
		while (!tcb->exited) {
			assert(cond_wait(&tcb->signal, &tcb->lock) == 0);
		}
		assert(mutex_unlock(&tcb->lock) == 0);

		/* Copy the joined thread's status. */
		if (statusp) {
			*statusp = tcb->statusp;
		}

		/* Deallocate the joined thread. */
		assert(mutex_destroy(&tcb->lock) == 0);
		assert(cond_destroy(&tcb->signal) == 0);
		free(tcb);
		return 0;
	}
	assert(mutex_unlock(&tid_table_lock) == 0);
	return -1;
}

tcb_t *thr_gettcb(boolean_t remove_tcb) {
	assert(initialized);
	//tcb_t tcb_struct;
	tcb_t *tcb = NULL; //&tcb_struct;
	char *stack_addr = get_addr();

	/* If our address is higher than the address of any child stack, then we must
	 * be the parent thread. */
	if (stack_addr > max_child_stack_addr) {
		return &main_thread;
	}

	unsigned int key = prehash(stack_addr);

	/* Our stack address will either use the same key as the top address of our
	 * stack, or it will be one greater. */
	assert(mutex_lock(&stack_table_lock) == 0);
	HASHTABLE_GET(hashtable_t, stack_table, key, tcb);

	/* If the top of the retrieved stack is above us, then we must be on the
	 * stack below the retrieved one. */
	if (!tcb || tcb->stack > stack_addr) {
		if (remove_tcb) {
			HASHTABLE_REMOVE(hashtable_t, stack_table, key - 1, tcb);
		}
		else {
			HASHTABLE_GET(hashtable_t, stack_table, key - 1, tcb);
		}
	}
	else if (remove_tcb) {
		HASHTABLE_REMOVE(hashtable_t, stack_table, key, tcb);
	}
	assert(mutex_unlock(&stack_table_lock) == 0);
	return tcb;
}

/** @brief Exit this thread with the given status
 *
 * @param status An opaque packet to pass to a waiting thread that called 
 *        thr_join on this thread
 */
void thr_exit(void *status) {
	assert(initialized);
	
	MAGIC_BREAK;
	/* Get tcb from stack table and remove it. */
	tcb_t *tcb = thr_gettcb(TRUE /* remove_tcb */);

	/* Set our status. */
	tcb->statusp = status;
	assert(mutex_lock(&tcb->lock) == 0);
	tcb->exited = TRUE;
	assert(mutex_unlock(&tcb->lock) == 0);

	if (tcb == &main_thread) {
		/* If we're the main thread, we can simply disappear. */
		assert(cond_signal(&tcb->signal) == 0);
		vanish();
	}
	else {
		/* Otherwise we must free our stack. We call free from the stack we are
		 * deallocating, so we must jump to the kill_stack dedicated for this
		 * purpose. */
		assert(mutex_lock(&kill_stack_lock) == 0);
		switch_to_stack(kill_stack);
		free(tcb->stack);
		assert(cond_signal(&tcb->signal) == 0);

		/* After unlocking the kill_stack mutex, we must vanish immediately without
		 * touching the stack again. This means we can't even try to return from
		 * mutex_unlock. */
		mutex_unlock_and_vanish(&kill_stack_lock);
	}
	// Shouldn't reach here
	assert(FALSE);
}

/** @brief Return the thread ID of the currently running thread
 *
 * @return the current thread's ID
 */
int thr_getid(void) {
	tcb_t *tcb = thr_gettcb(FALSE /* don't remove_tcb */);
	assert(tcb);
	return tcb->tid;
}

/** @brief Defer execution of this thread in favor of another
 *
 * @param tid The thread ID of the thread to defer to. If tid is -1, defer to
 *        any thread. It is an error if the given thread is not runnable
 *
 * @return 0 on success, less than 0 on error
 */
int thr_yield(int tid) {
	return yield(tid);
}

