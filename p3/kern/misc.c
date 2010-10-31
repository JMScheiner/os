#include <simics.h>
#include <reg.h>
#include <asm_helper.h>

/**
 * @brief Halt the system
 *
 * @param reg Ignored
 */
void halt_handler(volatile regstate_t reg)
{
   sim_halt();
   halt();
}

void ls_handler(volatile regstate_t reg)
{
   lprintf("ls handler not yet implemented.");
   MAGIC_BREAK;
}


