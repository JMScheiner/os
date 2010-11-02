/** 
* @file timer.c
*
* @brief Handles timer interrupts, and reports the number of ticks
* 	since system start.  _Not_ an effective clock, just a timer.
*
* @author Justin Scheiner
* @bug Timer drifts by about six seconds per day.
* @bug Timer handler needs to be double checked for race conditions.
*/

#include "timer.h"
#include <interrupt_defines.h>
#include <timer_defines.h>
#include <stdint.h>
#include <atomic.h>
#include <simics.h>
#include <asm.h>
#include <types.h>
#include <scheduler.h>
#include <global_thread.h>
#include <asm_helper.h>
#include <thread.h>
#include <reg.h>

//In reality this is 9.99931276 milliseconds.
//	Per tick, we lose 687.24ns.
//	That is about six seconds per day...
#define TEN_MS_MSB ((unsigned char)(((TIMER_RATE / 100) & 0xff00) / 4))
#define TEN_MS_LSB ((unsigned char)(TIMER_RATE / 100) & 0xff)

// This overflows every 12 hours - but is fine for the purposes of 15410. 
static volatile unsigned int ticks;

/** 
* @brief Initializes the timer. 
*/
void timer_init()
{
	ticks = 0;

	//Indicate that the timer should be in square wave mode.
	outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);

	//Write bytes to get 10ms interrupts generated.
	outb(TIMER_PERIOD_IO_PORT, TEN_MS_LSB);
	outb(TIMER_PERIOD_IO_PORT, TEN_MS_MSB);
}

/** 
* @brief Increments the timer counter. 
*/
void timer_handler(regstate_t reg)
{
   atomic_add_volatile(&ticks, 1);
	outb(INT_CTL_PORT, INT_ACK_CURRENT);
   
   /* Identify ourselves, and run the next thread. */
   tcb_t* global = global_tcb();
   if(get_esp() <= global->kstack)
   {
      scheduler_next(global_tcb());
   }
   else
   {
      scheduler_next(get_tcb());
   }
   
   /* If this fails then we've probably corrupted a 
    *  kernel stack. */
   assert(reg.eip);
}

/** 
* @brief Main access to the timer's current value.
* 
* @return The number of ticks since the handler has been installed.
*/
long time(void)
{
	return ticks;
}


