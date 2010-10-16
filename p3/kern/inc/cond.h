
#ifndef COND_H_HDGW65321F
#define COND_H_HDGW65321F

#include <mutex.h>

typedef struct cond {
	mutex_t lock;
} cond_t;

int cond_init(cond_t *cv);
int cond_destroy(cond_t *cv);
int cond_wait(cond_t *cv, mutex_t *mp);
int cond_signal(cond_t *cv);
int cond_broadcast(cond_t *cv);

#endif

