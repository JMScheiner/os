/** 
* @file mutex.h
* @brief Definitions for mutexes and quicklocks. 
*
* Quicklocks encompass most of the non-preemptibility in our kernel. 
*
* @author Tim Wilson
* @author Justin Scheiner
* @date 2010-11-12
*/

#ifndef MUTEX_H_GYTF3123G
#define MUTEX_H_GYTF3123G

#include <kernel_types.h>
#include <types.h>

extern boolean_t locks_enabled;

void mutex_init(mutex_t *mp);
void mutex_destroy(mutex_t *mp);
void mutex_lock(mutex_t *mp);
void mutex_unlock(mutex_t *mp);
void quick_lock();
void quick_unlock();
void quick_unlock_all();
void quick_fake_unlock();
void quick_assert_unlocked();
void quick_assert_locked();

#endif
