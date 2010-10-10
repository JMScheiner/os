
#include <reg.h>
#include <thread.h>
#include <simics.h>

void gettid_handler(volatile regstate_t reg)
{
   reg.eax = get_tcb()->tid;
}


