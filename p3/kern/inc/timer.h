
#ifndef TIMER_39YOD2E4
#define TIMER_39YOD2E4

#include <types.h>

void timer_init(void);

/** 
* @brief A basic wrapper for the timer handler.
* 	- Pushes all registers.
* 	- Calls timer handler.
* 	- Pops all registers.
*/
void asm_timer_wrapper(void);
long time(void);

#endif /* end of include guard: TIMER_39YOD2E4 */
