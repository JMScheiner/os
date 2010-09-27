/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include <cond.h>
#include <mutex.h>

typedef struct sem {
	int count;
	mutex_t lock;
	cond_t nonzero;
	
	int id;
	boolean_t initialized;
} sem_t;

#endif /* _SEM_TYPE_H */
