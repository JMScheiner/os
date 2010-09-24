/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <mutex_type.h>

typedef struct cond
{
  /* fill this in */
	int c;
} cond_t;

int cond_init(cond_t *cv);
int cond_destroy(cond_t *cv);
int cond_wait(cond_t *cv, mutex_t *mp);
int cond_signal(cond_t *cv);
int cond_broadcast(cond_t *cv);

#endif /* _COND_TYPE_H */
