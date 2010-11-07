
#include <mutex.h>
#include <x86/asm.h>
#include <atomic.h>
#include <timer.h>
#include <assert.h>
#include <types.h>
#include <thread.h>
#include <stdlib.h>
#include <scheduler.h>
#include <debug.h>
#include <eflags.h>

/**
 * @brief Global count of how many times interrupts have been disabled
 * without being enabled. This is intially 1 because initially interrupts
 * are disabled.
 */
int lock_depth = 1;

/** @brief Global flag preventing use of mutexes until the kernel has been
 * initialized. */
boolean_t locks_enabled = FALSE;

static void reset_lock_depth(int depth);

/**
 * @brief Initialize a mutex.
 *
 * @param mp The mutex to initialize.
 */
void mutex_init(mutex_t *mp) {
	assert(mp);

	mp->head = mp->tail = NULL;
	mp->initialized = TRUE;
	mp->locked = FALSE;
}

/**
 * @brief Mark a mutex as destroyed. Attempts to use a destroyed mutex will
 * cause an assertion to fail.
 *
 * @param mp The mutex to destroy.
 */
void mutex_destroy(mutex_t *mp) {
	assert(mp);
	assert(mp->initialized);
	assert(mp->locked == FALSE);
	mp->initialized = FALSE;
}

/**
 * @brief Lock a mutex to protect a critical section of code.
 *
 * If the lock cannot be obtained immediately, a context switch to the thread
 * holding the lock will be triggered.
 *
 * @param mp The mutex to lock.
 */
void mutex_lock(mutex_t *mp) 
{
	assert(mp);
	assert(mp->initialized);
	if (!locks_enabled) return;

	int depth = lock_depth;
	mutex_node_t node;
	node.tcb = get_tcb();
	node.next = NULL;
	debug_print("mutex", "Thread %p is entering mutex %p", node.tcb, mp);
	quick_lock();
	if (mp->head == NULL) {
		mp->head = mp->tail = &node;
	}
	else {
		mp->tail->next = &node;
		mp->tail = &node;
	}
	while (mp->locked || mp->head != &node) {
		scheduler_block();
		quick_lock();
	}
	mp->locked = TRUE;
	mp->head = mp->head->next;
	reset_lock_depth(depth);
	debug_print("mutex", "Thread %p has acquired mutex %p", node.tcb, mp);
}

/**
 * @brief Unlock a mutex after leaving a critical section.
 *
 * @param mp The mutex to unlock.
 */
void mutex_unlock(mutex_t *mp) {
	assert(mp);
	assert(mp->initialized);
	if (!locks_enabled) return;

	debug_print("mutex", "Releasing mutex %p", mp);
	
	mp->locked = FALSE;
	quick_lock();
	if(mp->head) scheduler_unblock(mp->head->tcb);
	quick_unlock();
}

/**
 * @brief Apply a global lock to the system that disables interrupts. This 
 * should only be used for fast operations that absolutely should not block.
 */
void quick_lock() {
	disable_interrupts();
	lock_depth++;
}

/**
 * @brief Release the global lock. The global lock can be locked multiple
 * times. Only enable interrupts if an equal number of unlocks have been
 * applied.
 */
void quick_unlock() {
	quick_assert_locked();
	if (--lock_depth == 0)
		enable_interrupts();
}

/**
 * @brief Release the global lock unconditionally and reset the number of
 * times it has been locked to 0.
 */
void quick_unlock_all() {
	quick_assert_locked();
	lock_depth = 0;
	enable_interrupts();
}

void quick_assert_locked() {
	assert(lock_depth > 0);
	assert((get_eflags() & EFL_IF) == 0);
}

static void reset_lock_depth(int depth) {
	quick_assert_locked();
	lock_depth = depth;
	if (lock_depth == 0)
		enable_interrupts();
}

