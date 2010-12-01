/** 
* @file timer.h
* @brief Definitions for the timer handler. 
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-11-12
*/

#ifndef TIMER_39YOD2E4
#define TIMER_39YOD2E4

#include <types.h>

void timer_init(void);

/** 
* @brief A basic wrapper for the timer handler.
*  - Pushes all registers.
*  - Calls timer handler.
*  - Pops all registers.
*/
void asm_timer_wrapper(void);
long get_time(void);

#endif /* end of include guard: TIMER_39YOD2E4 */
