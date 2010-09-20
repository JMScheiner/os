/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */



#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#define false 0
#define true 1
typedef unsigned int bool;

typedef volatile int tts_lock_t;

/* A simple test and test-and-set lock implementation. 
 * 	These will be useful later when doing the bounded
 * 	waiting version of mutexes, but are also a fair stand-in
 * 	until that code is written.
 */
int tts_lock(tts_lock_t* lock);
int tts_unlock(tts_lock_t* lock);
int tts_init(tts_lock_t* lock);
int tts_destroy(tts_lock_t* lock);

#endif /* THR_INTERNALS_H */


