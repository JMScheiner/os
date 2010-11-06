
#ifndef THREADMAN_4AB52XKO

#define THREADMAN_4AB52XKO

#include <reg.h>

void threadman_init();
void gettid_handler(volatile regstate_t reg);
void yield_handler(volatile regstate_t reg);
void deschedule_handler(volatile regstate_t reg);
void make_runnable_handler(volatile regstate_t reg);
void get_ticks_handler(volatile regstate_t reg);
void sleep_handler(volatile regstate_t reg);

#endif /* end of include guard: THREADMAN_4AB52XKO */



