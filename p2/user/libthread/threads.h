/** @file threads.h
 *
 * @brief Thread management API for user programs
 *
 * @author Tim Wilson
 */

int thr_init(unsigned int size);

int thr_create(void *(*func)(void *), void *arg);

int thr_join(int tid, void **statusp);

void thr_exit(void *status);

