#include <simics.h>
#include <reg.h>
#include <asm_helper.h>

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


