
#ifndef MUTEX_H_GYTF3123G
#define MUTEX_H_GYTF3123G

#include <thread_type.h>
#include <types.h>

extern boolean_t locks_enabled;

typedef struct mutex_node {
	tcb_t *tcb;
	struct mutex_node *next;
} mutex_node_t;

typedef struct mutex {
	mutex_node_t *head;
	mutex_node_t *tail;
	boolean_t locked;
	boolean_t initialized;
} mutex_t;

void mutex_init(mutex_t *mp);
void mutex_destroy(mutex_t *mp);
void mutex_lock(mutex_t *mp);
void mutex_unlock(mutex_t *mp);

#endif
