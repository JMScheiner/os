/** 
* @file timer.c
*
* @brief Handles timer interrupts, and reports the number of ticks
*  since system start.  _Not_ an effective clock, just a timer.
*
* @author Justin Scheiner
* @bug Timer drifts by about six seconds per day.
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
#include <ureg.h>
#include <mutex.h>

/* In reality this is 9.99931276 milliseconds.
   Per tick, we lose 687.24ns.
   That is about six seconds per day... */
#define TEN_MS_MSB ((unsigned char)(((TIMER_RATE / 100) & 0xff00) >> 8))
#define TEN_MS_LSB ((unsigned char)(TIMER_RATE / 100) & 0xff)

/** @brief The number of ticks since boot.
 * This overflows every 12 hours - but is fine for the purposes of 15410.
 */
static unsigned int ticks;

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
*
* @param reg Ignored
*/
void timer_handler(ureg_t* reg)
{
   ticks++;
   outb(INT_CTL_PORT, INT_ACK_CURRENT);

   /* Interrupts are disabled, so set the lock depth to 1 to indicate
    * this. */
   quick_lock();
   
   /* Run the next thread. */
   scheduler_next();
}

/** 
* @brief Main access to the timer's current value.
* 
* @return The number of ticks since the handler has been installed.
*/
long get_time(void)
{
   return ticks;
}


