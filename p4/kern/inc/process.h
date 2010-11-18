/** 
* @file process.h
* @brief Definitions for the PCB, and operations on it. 
*  Initialization, Destruction, etc.
* @author Tim Wilson
* @author Justin Scheiner
* @date 2010-11-12
*/

#ifndef PROCESS_HJSD67S
#define PROCESS_HJSD67S

#include <kernel_types.h>
#include <elf_410.h>
#include <types.h>

/* The starting address of the stack in all processes. */
#define USER_STACK_BASE 0xc0000000

extern pcb_t *init_process;

void free_process_resources(pcb_t* pcb, boolean_t vanishing);
int get_pid(void);

pcb_t* initialize_process(boolean_t first_process);
pcb_t* get_pcb(void);

#endif

