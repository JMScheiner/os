
#ifndef SYSCALL_CODES_FDKJ3489
#define SYSCALL_CODES_FDKJ3489

#define SYSCALL_SUCCESS 0
#define SYSCALL_INVALID_ARGS (-1)

#define READLINE_INVALID_LENGTH (-2)
#define READLINE_INVALID_BUFFER (-2)

/** @brief Error code indicating one of the string arguments is not in 
 * the user's memory region. */
#define EXEC_INVALID_ARG (-2)

/** @brief Error code indicating that the total size of 
 *    the arguments to exec is too large. */
#define EXEC_ARGS_TOO_LONG (-3)

/** @brief Error indicating the executable name could not be read. */
#define EXEC_INVALID_NAME (-4)

/** @brief Error indicating exec was called in a multithreaded process. */
#define EXEC_MULTIPLE_THREADS (-5)

/** @brief Error indicating there are no available children to 
 * wait on. */
#define WAIT_NO_CHILDREN (-2)

#define YIELD_NONEXISTENT (-2)

#define MAKE_RUNNABLE_NONEXISTENT (-2)
#define MAKE_RUNNABLE_SCHEDULED (-3)

#define REMOVE_PAGES_REGION_NOT_FOUND (-1)
#endif

