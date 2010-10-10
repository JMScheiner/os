
#ifndef THREAD_FORK_H
#define THREAD_FORK_H

int thread_fork(void *(*func)(void *), void *arg, char *stack_base, tcb_t *tcb);

#endif
