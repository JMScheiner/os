/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

#include <types.h>

typedef struct mutex {

	/* @brief The ticket counter, each thread is guaranteed a unique ticket. */
	int ticket;
	
	/* @brief The ticket of the thread that owns the mutex. */
	int now_serving;
	
	/* @brief The prehash of the thread that currently owns the mutex, 
	 * 		 or zero, if the mutex is free. */
	int owner_prehash;
	
	/* @brief Unique identifier for this mutex. */
	int id;
	
	/* @brief Flag indicating whether this mutex is initialized.*/
	boolean_t initialized;
	
} mutex_t;

#endif /* _MUTEX_TYPE_H */
