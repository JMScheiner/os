/** 
* @file handler.c
* @brief Installs general interrupt handlers.
* @author Justin Scheiner
* @bug None known.
*/
#include "handler.h"

#include <timer_defines.h>
#include <idt.h>
#include <handlers/handler_wrappers.h>
#include <keyhelp.h>
#include <string.h>
#include <asm.h>
#include <simics.h>
#include <stdio.h>


/** 
* @brief Responsible for setting up all interrupt handlers.
* 
* @param void (*tickback )(unsigned int) Callback function for timer.
* 
* @return zero on success.
*/
int handler_install()
{
	trap_gate_t tg;
	
   INSTALL_HANDLER(tg, asm_divide_error_handler, IDT_DE);
   INSTALL_HANDLER(tg, asm_debug_handler, IDT_DB);
   INSTALL_HANDLER(tg, asm_breakpoint_handler, IDT_BP);
   INSTALL_HANDLER(tg, asm_overflow_handler, IDT_OF);
   INSTALL_HANDLER(tg, asm_bound_range_exceeded_handler, IDT_BR);
   INSTALL_HANDLER(tg, asm_invalid_opcode_handler, IDT_UD);
   INSTALL_HANDLER(tg, asm_device_not_available_handler, IDT_NM);
	
	return 0;
}


