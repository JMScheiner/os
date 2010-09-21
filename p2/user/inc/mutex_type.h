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
	int held;
	int tid;
	int initialized;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
