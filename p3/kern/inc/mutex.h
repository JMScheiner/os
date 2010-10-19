
#ifndef MUTEX_H_GYTF3123G
#define MUTEX_H_GYTF3123G

#include <types.h>

extern boolean_t locks_enabled;
typedef struct mutex mutex_t;

struct mutex {
	/* TODO See if we can get includes right to make this a tcb_t *. */
	void *owner;
	int ticket;
	int now_serving;
	boolean_t initialized;
};

void mutex_init(mutex_t *mp);
void mutex_destroy(mutex_t *mp);
void mutex_lock(mutex_t *mp);
void mutex_unlock(mutex_t *mp);

#endif
