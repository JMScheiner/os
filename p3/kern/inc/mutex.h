
#ifndef MUTEX_H_GYTF3123G
#define MUTEX_H_GYTF3123G

typedef struct mutex {
	int id;
} mutex_t;

int mutex_init(mutex_t *mp);
int mutex_destroy(mutex_t *mp);
int mutex_lock(mutex_t *mp);
int mutex_unlock(mutex_t *mp);

#endif
