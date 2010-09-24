/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

#include <types.h>

typedef struct _mnode mutex_node;

typedef struct mutex 
{
	int initialized;
	int ticket;
	int now_serving;
	int active_tid;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
