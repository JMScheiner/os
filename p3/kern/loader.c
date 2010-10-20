/**
 * The 15-410 kernel project.
 * @name loader.c
 *
 * Functions for the loading
 * of user programs from binary 
 * files should be written in
 * this file. The function 
 * elf_load_helper() is provided
 * for your use.
 */
/*@{*/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>
#include <simics.h>
#include <scheduler.h>
#include <cr.h>

#include <process.h>
#include <thread.h>
#include <context_switch.h>
#include <mode_switch.h>
#include <eflags.h>
#include <assert.h>

/* --- Local function prototypes --- */ 


/**
 * Copies data from a file into a buffer.
 *
 * @param filename   the name of the file to copy data from
 * @param offset     the location in the file to begin copying from
 * @param size       the number of bytes to be copied
 * @param buf        the buffer to copy the data into
 *
 * @return returns the number of bytes copied on success; -1 on failure
 */
int getbytes( const char *filename, int offset, int size, char *buf ) {
	int i;
	int bytes;
	for (i = 0; i < exec2obj_userapp_count; i++) {
		exec2obj_userapp_TOC_entry file_entry = exec2obj_userapp_TOC[i];
		if (strcmp(file_entry.execname, filename) == 0) {
			bytes = (size < file_entry.execlen - offset) 
				? size 
				: file_entry.execlen - offset;
			memcpy(buf, file_entry.execbytes + offset, bytes);
			return bytes;
		}
	}

  return -1;
}

/** @brief Set a flag in a bit vector. */
#define SET(bit_vector, flag) \
	bit_vector |= flag

/** @brief Unset a flag in a bit vector. */
#define UNSET(bit_vector, flag) \
	bit_vector &= ~flag

/**
 * @brief Align an address by rounding up to an alignment boundary.
 *
 * @param addr The address to align.
 * @param align The number of bytes to align to.
 *
 * @return The aligned address.
 */
#define ALIGN_UP(addr, align) \
	(void *)((((unsigned int)(addr) + (align) - 1) / (align)) * (align))

/**
 * @brief Align an address by rounding down to an alignment boundary.
 *
 * @param addr The address to align.
 * @param align The number of bytes to align to.
 *
 * @return The aligned address.
 */
#define ALIGN_DOWN(addr, align) \
	(void *)(((unsigned int)(addr) / (align)) * (align))

/**
 * @brief Get a value for the eflags register suitable for use in user 
 * mode.
 *
 * @return An eflags value.
 */
unsigned int get_user_eflags() 
{
	unsigned int eflags = get_eflags();
	SET(eflags, EFL_RESV1);
	SET(eflags, EFL_IF);
	UNSET(eflags, EFL_IOPL_RING3);
	UNSET(eflags, EFL_AC);
	return eflags;
}

/**
 * @brief Copy the arguments to the new program to the user stack
 *
 * @param argc The number of arguments
 * @param argv A character array of \0 separated string arguments
 * @param arg_len The total number of bytes in argv, including all the \0
 *                bytes. 
 *
 * @return The base of the user stack.
 */
void *copy_to_stack(int argc, char *argv, int arg_len) {
	char *ptr = (char *)USER_STACK_BASE;
	char *args = ptr - arg_len;

	/* Copy the value of the arguments onto the stack. */
	memcpy(args, argv, arg_len);

	/* Move ptr to the address of argc on the user stack. */
	ptr = ALIGN_DOWN(args, sizeof(void *));
	ptr -= sizeof(char *) * (argc + 1) + sizeof(int) + sizeof(char **);

	/* The stack base is one word below argc. */
	void *user_stack = ptr - sizeof(void *);
	*(int *)ptr = argc;
	ptr += sizeof(int);
	*(char ***)ptr = (char **)(ptr + 4);
	ptr += sizeof(char **);

	/* Copy the address of each argument to form the argv array. */
	int i;
	for (i = 0; i < argc; i++) {
		*(char **)ptr = args;
		ptr += sizeof(char *);
		args += strlen(args) + 1;
	}

	/* argv must be NULL terminated. */
	*(char **)ptr = NULL;
	return user_stack;
}

/** @brief Load a new task from a file
 *
 * @param argc The number of arguments to the program
 * @param argv A \0 separated array of string arguments to the program. The
 *             first argument is the name of the program.
 * @param arg_len The total number of bytes in argv, including all the \0
 *                bytes. 
 *
 * @return < 0 on error. Never returns on success.
 */
int load_new_task(char *exec, int argc, char *argv, int arg_len) {
	int err;
	if ((err = elf_check_header(exec)) != ELF_SUCCESS) {
		return err;
	}

	// TODO checking and loading the elf header should happen separately so exec
	// can do it before freeing the current process.
	simple_elf_t elf_hdr;
	if ((err = elf_load_helper(&elf_hdr, exec)) != ELF_SUCCESS) {
		return err;
	}
   
	pcb_t* pcb = initialize_first_process();
   
   set_cr3((int)pcb->page_directory);
	if ((err = initialize_memory(exec, elf_hdr, pcb)) != 0) {
		return err;
	}
	tcb_t* tcb = initialize_thread(pcb);

	void *stack = copy_to_stack(argc, argv, arg_len);

	unsigned int user_eflags = get_user_eflags();
   scheduler_register(tcb);
	 lprintf("Running %s", exec);
	 MAGIC_BREAK;
	mode_switch(tcb->esp, stack, user_eflags, (void *)elf_hdr.e_entry);

	// Never get here
	assert(0);
	return 0;
}

/*@}*/
