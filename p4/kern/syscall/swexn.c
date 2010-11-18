
#include <reg.h>
#include <ecodes.h>
#include <simics.h>

void swexn_handler(volatile regstate_t reg)
{
   RETURN(ESUCCESS);
}



