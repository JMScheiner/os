
#ifndef SCHEDULER_JIJV6ZY3
#define SCHEDULER_JIJV6ZY3

#include <kernel_types.h>
#include <types.h>

void scheduler_init();
void scheduler_register(tcb_t* tcb);

boolean_t scheduler_run(tcb_t* tcb, mutex_t *lock);
void scheduler_block();
void scheduler_unblock(tcb_t* tcb);
void scheduler_deschedule(mutex_t *lock);
boolean_t scheduler_reschedule(tcb_t *tcb);
void scheduler_die(mutex_t *lock);
void scheduler_next();
void scheduler_sleep(unsigned long ticks);

// heap_t* scheduler_sleep_heap = NULL;

#endif /* end of include guard: SCHEDULER_JIJV6ZY3 */


