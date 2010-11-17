/** @file cond.c
 *
 * @brief A very basic condition variable that supports only one waiting
 * thread.
 *
 * @author Tim Wilson (tjwilson)
 * @author Justin Scheiner (jscheine)
 */
#ifndef COND_H_HDGW65321F
#define COND_H_HDGW65321F

#include <kernel_types.h>

void cond_init(cond_t *cv);
void cond_destroy(cond_t *cv);
void cond_wait(cond_t *cv);
void cond_signal(cond_t *cv);

#endif

