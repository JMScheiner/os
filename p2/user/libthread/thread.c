/** @file threads.c
 *
 * @brief Thread management API for user programs
 *
 * @author Tim Wilson
 */

#include <thread.h>

#include <mutex.h>
#include <cond.h>
#include <list.h>
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

/* @brief Really this needs to be 24 for the 6 words that get put on the user stack 
 * 	before being copied to the kernel stack by INT. */
#define INT_STACK_SIZE 32

/** @brief The size of a child stack. Initialized in thr_init. */
static unsigned int user_stack_size;

static unsigned int alloc_stack_size;

/** @brief Has thr_init been called? */
static boolean_t initialized = FALSE;

/** @brief The logical top of the shared kill stack. */
static char kill_stack_top[KILL_STACK_SIZE + 2*ESP_ALIGN - 1];

/** @brief The base of the shared kill stack. A thread jumps to this stack just
 * before it exits so it can free its own stack. */
static char *kill_stack;

/* @brief A safe place for traps to execute, so we don't overwrite part of 
 * the kill stack. */
static char int_stack_top[INT_STACK_SIZE + 2*ESP_ALIGN - 1];
static char* int_stack;

/** @brief A lock on the kill stack. */
static mutex_t kill_stack_lock;

static tcb_t *kill_stack_tcb = NULL;

/** @brief Define a hashtable type from int to tcb_t *. */
DEFINE_HASHTABLE(hashtable_t, int, tcb_t *);

/** @brief A table mapping tids to thread control blocks. The main parent
 * thread will be in this table. */
static hashtable_t tid_table;

/** @brief A lock for the tid_table. */
static mutex_t tid_table_lock;

/** @brief The maximum stack address a child thread could have. */
static char *max_child_stack_addr = NULL;

/** @brief A lock to synchronize writes to the maximum stack address. */
static mutex_t max_child_stack_addr_lock;

/** @brief The thread control block for the main parent thread. */
static tcb_t main_thread;

/** @brief Identity hash function
 *
 * @param key The key to hash
 *
 * @return The hash value
 */
static unsigned int hash(int key) {
	return (unsigned int)key;
}

#define ALIGN_DOWN_TCB(address) \
	(user_stack_size * ((unsigned int)(address) / user_stack_size))

#define ALIGN_UP_TCB(address) \
	(user_stack_size * (((unsigned int)(address) + user_stack_size - 1) / user_stack_size))

#define ALIGN_DOWN(address) \
	(((unsigned int)(address)) & ~(ESP_ALIGN - 1))

#define ALIGN_UP(address) \
	((((unsigned int)(address) - 1) | (ESP_ALIGN - 1)) + 1)

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

	user_stack_size = size + sizeof(tcb_t *);
	alloc_stack_size = ALIGN_UP(2 * user_stack_size);

	/* Initialize the main_thread block. */
	main_thread.stack = NULL;
	main_thread.tid = gettid();

	mutex_debug_print("Initializing main thread lock...");
	ret |= mutex_init(&(main_thread.lock));

	mutex_debug_print("Initializing main thread condition variables...");
	ret |= cond_init(&(main_thread.init_signal));
	ret |= cond_init(&(main_thread.exit_signal));

	main_thread.initialized = TRUE;
	main_thread.exited = FALSE;

	/* Initialize the tid table and its lock. */
	STATIC_INIT_HASHTABLE(hashtable_t, tid_table, hash);

	thread_debug_print("Main: inserting %d -> %p into tid table.", main_thread.tid, &main_thread);
	HASHTABLE_PUT(hashtable_t, tid_table, main_thread.tid, &main_thread);
	
	mutex_debug_print("Initializing tid table lock...");
	ret |= mutex_init(&tid_table_lock);
	
	/* Initialize the max stack address lock. */
	mutex_debug_print("Initializing max child stack address lock...");
	ret |= mutex_init(&max_child_stack_addr_lock);

	/* Initialize the kill stack and its lock. */
	kill_stack = kill_stack_top + KILL_STACK_SIZE + ESP_ALIGN - 1;
	kill_stack = (char *) ALIGN_DOWN(kill_stack);
	
	mutex_debug_print("Initializing kill stack lock...");
	ret |= mutex_init(&kill_stack_lock);

	/* Initialized the "int" stack. Since interrupts are atomic, 
	 * 	and all information gets copied to the kernel stack, no need to lock. */
	int_stack = int_stack_top + INT_STACK_SIZE + ESP_ALIGN - 1;
	int_stack = (char *) ALIGN_DOWN(int_stack);
	
	assert(ret == 0);
	return ret;
}

/************************ Life cycle of a thread **************************
 * Parent                              * Child                            *
 **************************************************************************
 * Initializes child tcb in thr_create *                                  *
 * Call thr_fork                       *                                  *
 * Wait for child to init              * Jump to new stack                *
 *                                     * Call thr_child_init              *
 *                                     * Get tid, add self to tables      *
 *                                     * Signal parent                    *
 * Continue                            * Call func                        *
 * ...                                 * ...                              *
 * Wait for child to die               * Call thr_exit                    *
 *                                     * Set status                       *
 *                                     * Jump to kill stack               *
 *                                     * Free own stack, signal exit      *
 * Get status, free child              * Jump to int stack                *
 *                                     * Call vanish                      *
 **************************************************************************/

/** @brief Create a new thread running the given function
 *
 * @param func The function the new thread will begin running
 *
 * @param arg The sole argument to func
 *
 * @return The thread ID of the new thread on success, less than 0 on error
 */
int thr_create(void *(*func)(void *), void *arg) 
{
	/*lprintf("Creating child\n");*/
	assert(initialized);
	int ret = -1;
	
	/* Create a thread control block and initialize its stack, mutex, and
	 * condition variable. */
	tcb_t *tcb = (tcb_t *)calloc(1, sizeof(tcb_t));
	assert(tcb);
	tcb->exited = FALSE;
	tcb->initialized = FALSE;
	
	mutex_debug_print("Initializing new tcb lock...");
	if(mutex_init(&tcb->lock)) {
		goto fail_mutex;
	}
	mutex_debug_print("Initializing new tcb condition variable...");
	if (cond_init(&tcb->init_signal)) {
		goto fail_init_cond;
	}
	if (cond_init(&tcb->exit_signal)) {
		goto fail_exit_cond;
	}
	tcb->stack = (char *)malloc(alloc_stack_size);
	assert(tcb->stack);

	/* Compute the base address of the child stack and place a pointer to the tcb
	 * above it. */
	char *stack_base = tcb->stack + alloc_stack_size - 4;
	stack_base = (char *)ALIGN_DOWN_TCB(stack_base);
	*(tcb_t **)stack_base = tcb;
/*	lprintf("New tcb at %p in [%p, %p]", stack_base, tcb->stack, tcb->stack + alloc_stack_size);*/
	stack_base -= 4;
	
	/* Update the max child stack address. */
	assert(mutex_lock(&max_child_stack_addr_lock) == 0);
	if (max_child_stack_addr < (char *)stack_base) {
		max_child_stack_addr = (char *)stack_base;
	}
	assert(mutex_unlock(&max_child_stack_addr_lock) == 0);
	
	/* Fork the new child thread. If we fail, undo all the initialization we've
	 * done. */
	ret = thread_fork(func, arg, stack_base, tcb);
/*	lprintf("[%d] Got back %d from thread_fork.", thr_getid(), ret);*/
	if (ret >= 0) {
		return ret;
	}
	lprintf(" ******** Failed to create child thread ******** ");

	free(tcb->stack);
	assert(cond_destroy(&tcb->exit_signal) == 0);
fail_exit_cond:
	lprintf(" ******** Failed to initialized condition variable ******* ");
	assert(cond_destroy(&tcb->init_signal) == 0);
fail_init_cond:
	lprintf(" ******** Failed to initialized condition variable ******* ");
	assert(mutex_destroy(&tcb->lock) == 0);
fail_mutex:
	lprintf(" ******** Failed to initialized mutex variable ******* ");
	free(tcb);
	return ret;
}

/** @brief Initialize a new child thread. 
 *
 * The tid still needs to be fetched, and the child needs to add its tcb to the
 * tid table.
 *
 * @param tcb The partial thread control block of the thread. 
 */
void thr_child_init(void *(*func)(void*), void* arg, tcb_t* tcb) {
	assert(tcb);
/*	lprintf("Child tcb is %p\n", tcb);*/
/*	lprintf("Child tcb is also %p\n", tcb2);*/
	
	tcb->tid = gettid();
	
	assert(mutex_lock(&tid_table_lock) == 0);
	thread_debug_print("Thread: inserting %d -> %p into tid table.", tcb->tid, tcb);
	HASHTABLE_PUT(hashtable_t, tid_table, tcb->tid, tcb);
	assert(mutex_unlock(&tid_table_lock) == 0);
	
	/* Notify the parent that we've fully initialized ourself. */
	assert(mutex_lock(&tcb->lock) == 0);
	tcb->initialized = TRUE;
	assert(mutex_unlock(&tcb->lock) == 0);
	assert(cond_signal(&tcb->init_signal) == 0);
	thread_debug_print("[%d] About to enter \"func\".", thr_getid());
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
/*	lprintf("Waiting on tcb %p\n", tcb);*/
	assert(mutex_lock(&tcb->lock) == 0);

	while (!tcb->initialized) 
	{
		assert(cond_wait(&tcb->init_signal, &tcb->lock) == 0);
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
	tcb_t *tcb = NULL;

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
			assert(cond_wait(&tcb->exit_signal, &tcb->lock) == 0);
		}
		assert(mutex_unlock(&tcb->lock) == 0);

		/* If the thread is inturrupted between setting exited to TRUE and
		 * signalling us AND we are made runnable, we could get here before the
		 * child calls cond_signal. The thread will have the kill_stack locked, so
		 * lock the kill_stack so we don't free our child before they cond_signal.
		 */
		assert(mutex_lock(&kill_stack_lock) == 0);

		/* Copy the joined thread's status. */
		if (statusp) {
			*statusp = tcb->statusp;
		}

		/* Deallocate the joined thread. */
		assert(mutex_destroy(&tcb->lock) == 0);
		assert(cond_destroy(&tcb->init_signal) == 0);
		assert(cond_destroy(&tcb->exit_signal) == 0);
		free(tcb);
		
		assert(mutex_unlock(&kill_stack_lock) == 0);
		return 0;
	}
	assert(mutex_unlock(&tid_table_lock) == 0);
	return -1;
}

/** @brief Get our tcb from the stack_list
 *
 * @return A pointer to our tcb
 */
tcb_t *thr_gettcb() {
	assert(initialized);
	char *stack_addr = get_addr();

	/* If this is being called from the kill stack, return the tcb of the
	 * thread using the kill stack. */
	if (kill_stack_top <= stack_addr && stack_addr <= kill_stack) {
		return kill_stack_tcb;
	}

	/* If our address is higher than the address of any child stack, then we must
	 * be the parent thread. */
	if (stack_addr > max_child_stack_addr) {
		return &main_thread;
	}

	tcb_t **tcb_addr = (tcb_t **)ALIGN_UP_TCB(stack_addr);
	return *tcb_addr;
}

/** @brief Free our own stack and vanish
 *
 * @param tcb Our thread control block
 */
void clean_up_thread(tcb_t *tcb) 
{
	kill_stack_tcb = tcb;
	free(tcb->stack);
	assert(mutex_lock(&tcb->lock) == 0);
	tcb->exited = TRUE;
	assert(mutex_unlock(&tcb->lock) == 0);
	assert(cond_signal(&tcb->exit_signal) == 0);

	/* After unlocking the kill_stack mutex, we must vanish immediately without
	 * touching the stack again. This means we can't even try to return from
	 * mutex_unlock. Use the int_stack to handle information put on the user stack 
	 * by INT */
	mutex_unlock_and_vanish(&kill_stack_lock, int_stack);
}

/** @brief Exit this thread with the given status
 *
 * @param status An opaque packet to pass to a waiting thread that called 
 *        thr_join on this thread
 */
void thr_exit(void *status) {
	assert(initialized);
	
	/* Get tcb from stack table and remove it. */
	tcb_t *tcb = thr_gettcb();

	/* Set our status. */
	tcb->statusp = status;

	if (tcb == &main_thread) {
		/* If we're the main thread, we can simply disappear. */
		assert(mutex_lock(&tcb->lock) == 0);
		tcb->exited = TRUE;
		assert(mutex_unlock(&tcb->lock) == 0);
		assert(cond_signal(&tcb->exit_signal) == 0);
		vanish();
	}
	else 
	{
		/* Otherwise we must free our stack. We call free from the stack we are
		 * deallocating, so we must jump to the kill_stack dedicated for this
		 * purpose. */
		/*lprintf("Before kill stack lock\n");*/
		mutex_lock(&kill_stack_lock);
		/*lprintf("After kill stack lock\n");*/
		switch_stacks_and_vanish(tcb, kill_stack);
	}
	// Shouldn't reach here
	assert(FALSE);
}

/** @brief Return the thread ID of the currently running thread
 *
 * @return the current thread's ID
 */
int thr_getid(void) {
	tcb_t *tcb = thr_gettcb();
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

