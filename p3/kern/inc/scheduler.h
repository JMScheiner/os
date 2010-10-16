
#ifndef SCHEDULER_JIJV6ZY3

#define SCHEDULER_JIJV6ZY3

#include <process.h>

void scheduler_register(pcb_t* pcb);
void scheduler_block(pcb_t* pcb);
pcb_t* scheduler_next(void);
void scheduler_sleep(pcb_t* pcb, unsigned long ticks);
void scheduler_init(void);

// heap_t* scheduler_sleep_heap = NULL;

#endif /* end of include guard: SCHEDULER_JIJV6ZY3 */


