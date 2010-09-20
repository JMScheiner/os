/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

#include <thr_internals.h>

typedef struct mutex 
{
	int in_use;
	tcb_queue q;
	tts_lock_t qlock;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
