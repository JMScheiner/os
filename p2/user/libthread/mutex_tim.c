
#include <mutex_type.h>
#include <mutex.h>
#include <syscall.h>
#include <atomic.h>
#include <types.h>
#include <stddef.h>
#include <thread.h>
#include <simics.h>

/** @brief Spin on the mutex's test and set lock until it can be locked. */
#define LOCK(mutex, ticket) \
	ticket = 1; \
	atomic_xadd(&ticket, &mutex->last_ticket); \
	while (ticket != mutex->current_ticket) \
		yield(-1); \

/** @brief Unlock the mutex's test and set lock. */
#define UNLOCK(mutex) \
	mutex->current_ticket++

/** @brief Initialize a mutex before its first use.
 *
 * @param mp A pointer to the mutex.
 *
 * @return 0 on success, less than 0 on error.
 */
int mutex_init(mutex_t *mp) {
	if (!mp) return -1;
	mp->head = NULL;
	mp->tail = NULL;
	mp->free = TRUE;
	mp->initialized = TRUE;
	mp->current_ticket = 0;
	mp->last_ticket = 0;
	mp->user = -1;
	return 0;
}

/** @brief Destroy a mutex after its last use.
 *
 * @param mp A pointer to the mutex.
 *
 * @return 0 on success, less than 0 on error.
 */
int mutex_destroy(mutex_t *mp) {
	if (!mp) return -1;
	if (!mp->initialized) return -2;

	mp->initialized = FALSE;
	return 0;
}

/** @brief Lock the mutex, forcing others to wait until it is unlocked.
 *
 * @param mp A pointer to the mutex.
 *
 * @return 0 on success, less than 0 on error.
 */
int mutex_lock(mutex_t *mp) {
	if (!mp) return -1;
	if (!mp->initialized) return -2;

	mutex_node_t node;
	node.tid = thr_getid();
	node.next = NULL;
	int ticket;

	/* Acquire the lock and place our tid at the end of the waiting list 
	 * for the mutex. */
	LOCK(mp, ticket);
	if (mp->tail) {
		mp->tail->next = &node;
	}
	else {
		mp->head = &node;
	}
	mp->tail = &node;
	UNLOCK(mp);

	/* If we are not at the head of the list and the mutex is not being 
	 * released, then deschedule ourselves. */
	while (!mp->free || node.tid != mp->head->tid) {
		deschedule(&mp->free);
	}

	/* We have the mutex, so remove our tid from the head of the list */
	mp->free = FALSE;
	mp->user = node.tid;
	LOCK(mp, ticket);
	mp->head = mp->head->next;
	if (!mp->head) {
		mp->tail = NULL;
	}
	UNLOCK(mp);
	return 0;
}

/** @brief Unock the mutex, allowing another thread to proceed.
 *
 * @param mp A pointer to the mutex.
 *
 * @return 0 on success, less than 0 on error.
 */
int mutex_unlock(mutex_t *mp) {
	if (!mp) return -1;
	if (!mp->initialized) return -2;

	int ticket;

	/* Remove our tid from the head of the list, and make the next thread
	 * runnable. */
	mp->free = TRUE;
	LOCK(mp, ticket);
	if (mp->head)
		make_runnable(mp->head->tid);
	mp->user = -1;
	UNLOCK(mp);
	
	return 0;
}

