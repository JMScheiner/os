/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

/** 
* @brief Useful for stack based mutex waiting lists.
*/
//typedef struct _mnode { 
//	boolean_t cancel_deschedule;
//	struct _mnode* next_thread;
//} mutex_node;

typedef struct mutex {
	int initialized;
	int ticket;
	int now_serving;
	int active_tid;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
