
#include <mutex.h>
#include <x86/asm.h>
#include <atomic.h>
#include <timer.h>
#include <assert.h>
#include <types.h>
#include <thread.h>
#include <stdlib.h>
#include <scheduler.h>
#include <simics.h>

/**
 * @brief Global flag indicating whether locks have been enabled yet. Locks
 * cannot be enabled until interrupts are enabled.
 */
boolean_t locks_enabled = FALSE;

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
void mutex_lock(mutex_t *mp) {
	assert(mp);
	assert(mp->initialized);
	if (!locks_enabled) return;

	mutex_node_t node;
	node.tcb = get_tcb();
	node.next = NULL;
	disable_interrupts();
	if (mp->head == NULL) {
		mp->head = mp->tail = &node;
	}
	else {
		mp->tail->next = &node;
		mp->tail = &node;
	}
	while (mp->locked || mp->head != &node) {
		scheduler_run(mp->head->tcb);
		disable_interrupts();
	}
	enable_interrupts();
	mp->locked = TRUE;
	mp->head = mp->head->next;
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
	assert(mp->locked == TRUE);

	mp->locked = FALSE;
}

