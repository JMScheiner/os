/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

#include <types.h>

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

	/* True if the mutex is free. */
	int free;

	/* True iff the mutex has been initialized. */
	boolean_t initialized;

	/* The ticket currently being served. */
	int current_ticket;

	/* The last ticket not yet given out. */
	int last_ticket;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
