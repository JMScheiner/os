
#ifndef PROCESS_HJSD67S
#define PROCESS_HJSD67S

#include <kernel_types.h>
#include <elf_410.h>
#include <types.h>

/* The starting address of the stack in all processes. */
#define USER_STACK_BASE 0xc0000000

/* The last address we will allocate for the user. */
#define USER_STACK_START 0xb0000000

extern pcb_t *init_process;

int get_pid(void);

int initialize_memory(const char *file, simple_elf_t elf, pcb_t* pcb);
pcb_t* initialize_process(boolean_t first_process);
pcb_t* get_pcb(void);

#endif

