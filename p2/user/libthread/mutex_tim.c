
#include <mutex.h>
#include <syscall.h>
#include <atomic.h>
#include <types.h>
#include <stddef.h>
#include <thread.h>

/** @brief Spin on the mutex's test and set lock until it can be locked. */
#define LOCK(mutex) \
	while (atomic_cmpset(&mutex->lock, 1, 0) != 0)

/** @brief Unlock the mutex's test and set lock. */
#define UNLOCK(mutex) \
	mutex->lock = 0

/** @brief A node in the mutex's linked list. */
typedef struct mutex_node {
	/** @brief Tid of a thread waiting on this mutex. */
	int tid;

	/** @brief Next node in the list. */
	struct mutex_node *next;
} mutex_node_t;

/** @brief A mutex to allow exclusive access to a section of code. */
typedef struct mutex {
	/* The head node in the waiting list for this mutex. */
	mutex_node_t *head;

	/* The last node in the waiting list for this mutex. */
	mutex_node_t *tail;

	/* A test and set lock to protect the waiting list. */
	int lock;

	/* True if the mutex is free. */
	boolean_t free;

	/* True iff the mutex has been initialized. */
	boolean_t initialized;
} mutex_t;

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
	mp->lock = 0;
	mp->free = TRUE;
	mp->initialized = TRUE;
}

/** @brief Destroya mutex after its last use.
 *
 * @param mp A pointer to the mutex.
 *
 * @return 0 on success, less than 0 on error.
 */
int mutex_destroy(mutex_t *mp) {
	if (!mp) return -1;
	if (!mp->initialized) return -2;
	if (mp->lock != 0) return -3;

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
	LOCK(mp);
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
	while (!mp->free && node.tid != mp->head->tid) {
		deschedule((int *)&mp->free);
	}

	/* We have the lock, so remove our tid from the head of the list */
	mp->free = FALSE;
	LOCK(mp);
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

	/* Remove our tid from the head of the list, and make the next thread
	 * runnable. */
	mp->free = TRUE;
	if (mp->head)
		make_runnable(mp->head->tid);
	return 0;
}

