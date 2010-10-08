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
	
	/******** Timer handler ********/
   INSTALL_HANDLER(tg, asm_double_fault_handler, IDT_DF);
   INSTALL_HANDLER(tg, asm_invalid_tss_handler, IDT_TS);
	
	return 0;
}


