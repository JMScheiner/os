
#ifndef MUTEX_H_GYTF3123G
#define MUTEX_H_GYTF3123G

#include <kernel_types.h>
#include <types.h>

extern boolean_t locks_enabled;

void mutex_init(mutex_t *mp);
void mutex_destroy(mutex_t *mp);
void mutex_lock(mutex_t *mp);
void mutex_unlock(mutex_t *mp);

#endif
