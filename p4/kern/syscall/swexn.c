
#include <reg.h>
#include <ecodes.h>
#include <simics.h>
#include <ureg.h>

void swexn_handler(volatile regstate_t reg)
{
   RETURN(ESUCCESS);
}

/** 
* @brief Checks for a registered exception handler, 
*  and if it finds one, builds a context for the 
*  exception handler. 
* 
* @param ureg The register state on entry to the handler.
* 
* @return ESUCCESS if the context was successfully arranged. 
*/
int swexn_build_context(ureg_t* ureg)
{

   return EFAIL;
}




