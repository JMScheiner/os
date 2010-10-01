/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

#include <types.h>

/* Mutex error codes */
#define MUTEX_NULL -1
#define MUTEX_INIT -2
#define MUTEX_IN_USE -3

/** @brief A mutex structure */
typedef struct mutex {

	/* @brief The ticket counter, each thread is guaranteed a unique ticket. */
	int ticket;
	
	/* @brief The ticket of the thread that owns the mutex. */
	int now_serving;
	
	/* @brief The tid of the mutex owner, or NULL_TID if no one holds the 
	 * mutex. */
	int active_tid;
	
	/* @brief Unique identifier for this mutex. */
	int id;
	
	/* @brief Flag indicating whether this mutex is initialized.*/
	boolean_t initialized;
	
} mutex_t;

#endif /* _MUTEX_TYPE_H */
