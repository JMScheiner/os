
#include <mutex_type.h>
#include <mutex.h>
#include <syscall.h>
#include <atomic.h>
#include <types.h>
#include <stddef.h>
#include <thread.h>
#include <simics.h>

/** @brief Initialize a mutex before its first use.
 *
 * @param mp A pointer to the mutex.
 *
 * @return 0 on success, less than 0 on error.
 */
int mutex_init(mutex_t *mp) {
	if (!mp) return -1;
	mp->head.tid = -1;
	mp->head.next = NULL;
	mp->tail = &mp->head;
	mp->free = TRUE;
	mp->initialized = TRUE;
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

	/* Place our tid at the end of the waiting list for the mutex. */
	mutex_node_t *next = mp->tail->next;
	mutex_node_t *last = mp->tail;
	while (atomic_cmpxchg8b(mp->tail->next, mp->tail, &next, &last, &node))
		;

	/* If we are not at the head of the list and the mutex is not being 
	 * released, then deschedule ourselves. */
	while (!mp->free || node.tid != mp->head.next->tid) {
		deschedule(&mp->free);
	}

	/* We have the mutex, so remove our tid from the head of the list */
	mp->free = FALSE;
	mp->head.next = mp->head.next->next;
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

	mp->free = TRUE;
	if (mp->head.next) {
		make_runnable(mp->head.next->tid);
	}
	return 0;
}

