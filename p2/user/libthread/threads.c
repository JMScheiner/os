/** @file threads.c
 *
 * @brief Thread management API for user programs
 *
 * @author Tim Wilson
 */

#include <assert.h>
#include <mutex_type.h>
#include <syscall.h>
#include <threads.h>
#include <thr_internals.h>
#include <queue.h>

static unsigned int stack_size;
static bool initialized = false;

struct thread_status_block {
	int tid;
	void *status;
	mutex_t lock;
	cond_t signal;
	bool exited;
	struct thread_status_block *next;
	struct thread_status_block *prev;
};
typedef struct thread_status_block thread_status_block_t;

DEFINE_QUEUE(thread_queue_t, thread_status_block_t *);
static thread_queue_t thread_queue;
static mutex_t queue_lock;

/** @brief Initialize the thread library
 *
 * @param size The amount of stack space in bytes that each thread will
 *        have available
 *
 * @return 0 on success, less than 0 on error
 */
int thr_init(unsigned int size) {
	assert(!initialized);
	initialized = true;
	stack_size = size;
	STATIC_INIT_QUEUE(thread_queue);
	assert(mutex_init(&queue_lock) == 0);
	return -1;
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
	char *stack_top = (char *)malloc(stack_size + PAGE_SIZE);
	assert(stack_top);
	char *stack_bottom = stack_top + stack_size + PAGE_SIZE;
	stack_bottom &= PAGE_SIZE;
	int tid = thread_fork(func, arg, stack_bottom - 4);
	if (tid >= 0) {
		thread_status_block_t *tsb = 
			(thread_status_block_t *)malloc(sizeof(thread_status_block_t));
		assert(tsb);
		tsb->tid = tid;
		tsb->exited = false;
		assert(mutex_init(&tsb->lock) == 0);
		assert(cond_init(&tsb->signal) == 0);
		assert(mutex_lock(&queue_lock) == 0);
		ENQUEUE_FIRST(tsb);
		assert(mutex_unlock(&queue_lock) == 0);
	}
	return tid;
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
	thread_status_block_t *tsb;
	assert(mutex_lock(&queue_lock) == 0);
	FOREACH(thread_queue, tsb) {
		if (tsb->tid == tid) {
			DEQUEUE_ELEM(thread_queue, tsb);
			assert(mutex_unlock(&queue_lock) == 0);
			assert(mutex_lock(&tsb->lock) == 0);
			while (!tsb->exited) {
				assert(cond_wait(&tsb->signal, &tsb->lock) == 0);
			}
			assert(mutex_unlock(&tsb->lock) == 0);
			assert(mutex_destroy(&tsb->lock) == 0);
			assert(cond_destroy(&tsb->signal) == 0);
			if (statusp) {
				*statusp = tsb->statusp;
			}
			free(tsb);
			return 0;
		}
	}
	mutex_unlock(&queue_lock);
	return -1;
}

/** @brief Exit this thread with the given status
 *
 * @param status An opaque packet to pass to a waiting thread that called 
 *        thr_join on this thread
 */
void thr_exit(void *status) {
	assert(initialized);
	thread_status_block_t *tsb;
	int tid = thr_getid();
	assert(mutex_lock(&queue_lock) == 0);
	FOREACH(thread_queue, tsb) {
		if (tsb->tid == tid) {
			assert(mutex_unlock(&queue_lock) == 0);
			tsb->statusp = status;
			tsb->exited = true;
			assert(cond_signal(&tsb->signal) == 0);
			vanish();
			// Shouldn't reach here!
			assert(false);
			return;
		}
	}
	// Shouldn't reach here!
	assert(false);
	return;
}

/** @brief Return the thread ID of the currently running thread
 *
 * @return the current thread's ID
 */
int thr_getid(void) {
	return gittid();
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
