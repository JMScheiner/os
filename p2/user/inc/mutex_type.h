/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H


typedef struct mutex {
	tts_lock_t lock;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
