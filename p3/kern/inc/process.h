
#ifndef PROCESS_HJSD67S
#define PROCESS_HJSD67S

#include <kernel_types.h>
#include <elf_410.h>

#define USER_STACK_BASE 0xc0000000
#define USER_STACK_START 0xb0000000

void init_process_table(void);
int get_pid(void);

int initialize_memory(const char *file, simple_elf_t elf, pcb_t* pcb);
pcb_t* initialize_process();
pcb_t* initialize_first_process();
pcb_t* get_pcb(void);

#endif

