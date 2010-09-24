/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

typedef struct _mnode mutex_node;

typedef struct mutex {
	mutex_node* last;
	mutex_node* next;
	mutex_node* running;
	int held;
	int tid;
	int initialized;
} mutex_t;

int mutex_init(mutex_t *mp);
int mutex_destroy(mutex_t *mp);
int mutex_lock(mutex_t *mp);
int mutex_unlock(mutex_t *mp);

#endif /* _MUTEX_TYPE_H */
