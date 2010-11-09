/* The 15-410 kernel project
 *
 *     loader.h
 *
 * Structure definitions, #defines, and function prototypes
 * for the user process loader.
 */

#ifndef _LOADER_H_HWIF23R4
#define _LOADER_H_HWIF23R4

#include <elf_410.h>
#include <kernel_types.h>

/* --- Prototypes --- */

int getbytes( const char *filename, int offset, int size, char *buf );

/*
 * Declare your loader prototypes here.
 */

unsigned int get_user_eflags();
void *copy_to_stack(int argc, char *argv, int arg_len);
int get_elf(char *exec, simple_elf_t *elf_hdr);
void switch_to_user(tcb_t *tcb, char *exec, void *stack, void *eip);
int load_new_task(char *exec, int argc, char *argv, int arg_len);

#endif /* _LOADER_H */
