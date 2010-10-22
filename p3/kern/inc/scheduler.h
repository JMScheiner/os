
#ifndef SCHEDULER_JIJV6ZY3

#define SCHEDULER_JIJV6ZY3

#include <process.h>

void scheduler_init(void);
void scheduler_register(tcb_t* tcb);

void scheduler_run(tcb_t *tcb);
void scheduler_next(void);

void scheduler_block(tcb_t* tcb);
void scheduler_block_me(tcb_t* me);
void scheduler_make_runnable(tcb_t* tcb);

void scheduler_sleep(tcb_t* tcb, unsigned long ticks);

// heap_t* scheduler_sleep_heap = NULL;

#endif /* end of include guard: SCHEDULER_JIJV6ZY3 */


