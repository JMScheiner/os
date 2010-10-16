
#ifndef SCHEDULER_JIJV6ZY3

#define SCHEDULER_JIJV6ZY3

#include <process.h>

void scheduler_register(tcb_t* tcb);
void scheduler_block(tcb_t* tcb);
tcb_t* scheduler_next(void);
void scheduler_sleep(tcb_t* tcb, unsigned long ticks);
void scheduler_init(void);

// heap_t* scheduler_sleep_heap = NULL;

#endif /* end of include guard: SCHEDULER_JIJV6ZY3 */


