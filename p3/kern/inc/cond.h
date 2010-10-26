
#ifndef COND_H_HDGW65321F
#define COND_H_HDGW65321F

#include <kernel_types.h>

void cond_init(cond_t *cv);
void cond_destroy(cond_t *cv);
void cond_wait(cond_t *cv);
void cond_signal(cond_t *cv);

#endif

