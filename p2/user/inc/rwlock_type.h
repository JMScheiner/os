/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <types.h>
#include <cond.h>
#include <mutex.h>

typedef struct rwlock 
{
	int mode;
	int writers, readers;
	
	boolean_t clear;
	cond_t wait_read, wait_write; 
	mutex_t clear_lock;

	boolean_t initialized;

} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
