
#ifndef SWEXN_H_HUI234II
#define SWEXN_H_HUI234II

#include <ureg.h>

void swexn_try_invoke_handler(ureg_t* ureg);
void swexn_return(void *eip, unsigned int cs_reg, unsigned int eflags, 
      void *esp, unsigned int ss_reg);

#endif

