
#ifndef THREAD_FWFWJ23E
#define THREAD_FWFWJ23E

#define KERNEL_STACK_SIZE 1

#include <kernel_types.h>
#include <list.h>

extern hashtable_t tcb_table;

void free_thread_resources(tcb_t* tcb);
void thread_init(void);
tcb_t* initialize_thread(pcb_t *pcb);
tcb_t *get_tcb(void);

#endif

