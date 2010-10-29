

#ifndef LIFECYCLE_DP4QFXLY

#define LIFECYCLE_DP4QFXLY

#include <reg.h>

/******** Defines for exec *******/
 
/** @brief Maxixmum allowed size of the kernel stack to use for exec arguments.
 * Should be some constant fraction of the kernel stack. */
#define MAX_TOTAL_LENGTH ((KERNEL_STACK_SIZE * PAGE_SIZE) / 2)

#define MAX_NAME_LENGTH 127

/** @brief Error code indicating the arguments to exec are not in the user's
 *    memory region. */
#define EXEC_INVALID_ARGS -1

/** @brief Error code indicating one of the string arguments is not in the
 *    user's memory region. */
#define EXEC_INVALID_ARG -2

/** @brief Error code indicating that the total size of 
 *    the arguments to exec is too large. */
#define EXEC_ARGS_TOO_LONG -3
#define EXEC_INVALID_NAME -4

#define WAIT_INVALID_ARGS -1
#define WAIT_NO_CHILDREN -2

void lifecycle_init();

void exec_handler(volatile regstate_t reg);

void thread_fork_handler(volatile regstate_t reg);
void fork_handler(volatile regstate_t reg);
void* arrange_fork_context(void* esp, regstate_t* reg, void* page_directory);

void set_status_handler(volatile regstate_t reg);
void vanish_handler(volatile regstate_t reg);
void wait_handler(volatile regstate_t reg);
void task_vanish_handler(volatile regstate_t reg);

#endif /* end of include guard: LIFECYCLE_DP4QFXLY */



