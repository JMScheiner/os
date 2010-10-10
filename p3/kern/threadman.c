
#include <reg.h>
#include <thread.h>

void gettid_handler(regstate_t reg)
{
   reg.eax = get_tcb()->tid;
}


