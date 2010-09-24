/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

#include <types.h>

typedef struct _mnode mutex_node;

typedef struct mutex {
	mutex_node* last;
	mutex_node* next;
	mutex_node* running;
	boolean_t held;
	boolean_t initialized;
	int tid;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
