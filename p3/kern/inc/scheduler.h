
#ifndef SCHEDULER_JIJV6ZY3

#define SCHEDULER_JIJV6ZY3

#include <kernel_types.h>

void scheduler_init();
void scheduler_register(tcb_t* tcb);

void scheduler_run(tcb_t *tcb);
void scheduler_next(tcb_t* tcb);

void scheduler_block(tcb_t* tcb);
void scheduler_block_me();
void scheduler_make_runnable(tcb_t* tcb);
void scheduler_die(mutex_t *lock);

void scheduler_sleep(unsigned long ticks);

// heap_t* scheduler_sleep_heap = NULL;

#endif /* end of include guard: SCHEDULER_JIJV6ZY3 */


