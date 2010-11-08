

#ifndef LIFECYCLE_DP4QFXLY
#define LIFECYCLE_DP4QFXLY

#include <reg.h>

/******** Defines for exec *******/
 
/** @brief Maxixmum allowed size of the kernel stack to use for exec 
 * arguments. Should be some constant fraction of the kernel stack. */
#define MAX_TOTAL_LENGTH ((KERNEL_STACK_SIZE * PAGE_SIZE) / 6)
#define MAX_NAME_LENGTH 127
#define STATUS_KILLED -2

void lifecycle_init();

void exec_handler(volatile regstate_t reg);

void thread_kill(char* error_message);
void thread_fork_handler(volatile regstate_t reg);
void fork_handler(volatile regstate_t reg);
void* arrange_fork_context(void* esp, regstate_t* reg, void* page_directory);

void set_status_handler(volatile regstate_t reg);
void vanish_handler();
void wait_handler(volatile regstate_t reg);
void task_vanish_handler(volatile regstate_t reg);
void arrange_global_context(void);

#endif /* end of include guard: LIFECYCLE_DP4QFXLY */



