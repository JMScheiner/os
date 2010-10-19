
#include <mutex.h>
#include <x86/asm.h>
#include <atomic.h>
#include <timer.h>
#include <assert.h>
#include <types.h>
#include <thread.h>
#include <stdlib.h>
#include <scheduler.h>

boolean_t locks_enabled = FALSE;

void mutex_init(mutex_t *mp) {
	assert(mp);

	mp->owner = NULL;
	mp->ticket = 0;
	mp->now_serving = 0;
	mp->initialized = TRUE;
}

void mutex_destroy(mutex_t *mp) {
	assert(mp);
	assert(mp->initialized);
	mp->initialized = FALSE;
}

void mutex_lock(mutex_t *mp) {
	assert(mp);
	assert(mp->initialized);
	if (!locks_enabled) return;

	int ticket = atomic_add(&mp->ticket, 1);
	while (ticket != mp->now_serving) {
		context_switch_on_tick = FALSE;
		if (mp->owner != NULL)
			scheduler_run((tcb_t *)(mp->owner));
		else
			scheduler_next();
		context_switch_on_tick = TRUE;
	}

	mp->owner = get_tcb();
}

void mutex_unlock(mutex_t *mp) {
	assert(mp);
	assert(mp->initialized);
	if (!locks_enabled) return;

	mp->owner = NULL;
	mp->now_serving++;
}

