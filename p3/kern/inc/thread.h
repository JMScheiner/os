
#ifndef THREAD_FWFWJ23E
#define THREAD_FWFWJ23E

#define KERNEL_STACK_SIZE 1

#include <kernel_types.h>
#include <list.h>

int new_tid(void);
void init_thread_table(void);

void free_thread_resources(tcb_t* tcb);
tcb_t* initialize_thread(pcb_t *pcb);
tcb_t *get_tcb(void);

#endif

