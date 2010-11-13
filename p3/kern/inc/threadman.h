/** 
* @file threadman.h
* @brief Definitions for the thread management part of the spec. 
*  Generally user-space operations on the scheduler.
*
* @author Tim Wilson
* @author Justin Scheiner
* @date 2010-11-12
*/

#ifndef THREADMAN_4AB52XKO

#define THREADMAN_4AB52XKO

#include <reg.h>

void gettid_handler(volatile regstate_t reg);
void yield_handler(volatile regstate_t reg);
void deschedule_handler(volatile regstate_t reg);
void make_runnable_handler(volatile regstate_t reg);
void get_ticks_handler(volatile regstate_t reg);
void sleep_handler(volatile regstate_t reg);

#endif /* end of include guard: THREADMAN_4AB52XKO */



