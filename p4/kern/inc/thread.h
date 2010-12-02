/** 
* @file thread.h
* @brief Definitions for the TCB. 
*  Initialization, Destruction, etc. 
* @author Tim Wilson
* @author Justin Scheiner
* @date 2010-11-12
*/

#ifndef THREAD_FWFWJ23E
#define THREAD_FWFWJ23E

/** @brief Number of pages per kernel stack. */
#define KERNEL_STACK_SIZE 1

#include <kernel_types.h>
#include <types.h>

void free_thread_resources(tcb_t* tcb);
void thread_init(void);
tcb_t* initialize_thread(pcb_t *pcb);
tcb_t *get_tcb(void);
void check_invariants(boolean_t synchronous);
hashtable_t* tcb_table(void);

#endif

