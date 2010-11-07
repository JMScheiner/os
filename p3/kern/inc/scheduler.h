
#ifndef SCHEDULER_JIJV6ZY3
#define SCHEDULER_JIJV6ZY3

#include <kernel_types.h>
#include <types.h>

void scheduler_init();
void scheduler_register(tcb_t* tcb);

void scheduler_run(tcb_t *tcb);
void scheduler_block();
void scheduler_unblock(tcb_t* tcb);
void scheduler_deschedule();
boolean_t scheduler_reschedule(tcb_t *tcb);
void scheduler_die(mutex_t *lock, pcb_t *pcb);
void scheduler_next();
void scheduler_sleep(unsigned long ticks);

// heap_t* scheduler_sleep_heap = NULL;

#endif /* end of include guard: SCHEDULER_JIJV6ZY3 */


