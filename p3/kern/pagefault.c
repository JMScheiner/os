#include <reg.h>
#include <simics.h>

void page_fault_handler(regstate_t reg)
{
   lprintf("esp = %d", (int)reg.esp);
}

