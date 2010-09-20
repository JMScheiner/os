/** @file threads.c
 *
 * @brief Thread management API for user programs
 *
 * @author Tim Wilson
 */

#include <threads.h>

/** @brief Initialize the thread library
 *
 * @param size The amount of stack space in bytes that each thread will
 *        have available
 *
 * @return 0 on success, less than 0 on error
 */
int thr_init(unsigned int size) {
	return -1;
}

/** @brief Create a new thread running the given function
 *
 * @param func The function the new thread will begin running
 *
 * @param arg The sole argument to func
 *
 * @return The thread ID of the new thread on success, less than 0 on error
 */
int thr_create(void *(*func)(void *), void *arg) {
	return -1;
}

/** @brief Wait for the specified thread to exit and collect its status
 *
 * @param tid The ID of the thread to wait for. It is an error to pass a tid
 *        that has not yet been created by thr_create
 *
 * @param statusp If not NULL, the status returned by the exiting thread will
 *        be placed here
 *
 * @return 0 on success, less than 0 on error
 */
int thr_join(int tid, void **statusp) {
	return -1;
}

/** @brief Exit this thread with the given status
 *
 * @param status An opaque packet to pass to a waiting thread that called 
 *        thr_join on this thread
 */
void thr_exit(void *status) {
	return;
}

