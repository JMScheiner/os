/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

#include <thr_internals.h>

typedef struct mutex 
{
	mutex_node* last;
	mutex_node* next;
	mutex_node* running;
	boolean_t held;
	boolean_t initialized;
	int tid;
} mutex_t;

int mutex_unlock_and_vanish(mutex_t* mp);

#endif /* _MUTEX_TYPE_H */
