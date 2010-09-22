/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <thr_internals.h>
#include <mutex.h>

typedef struct cond 
{
	cond_queue_t q;
	mutex_t qlock;
} cond_t;

#endif /* _COND_TYPE_H */
