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

int load_new_task(const char *file) {
	int err;
	if ((err = elf_check_header(file)) != ELF_SUCCESS) {
		return err;
	}

	simple_elf_t elf_hdr;
	if ((err = elf_load_helper(&elf_hdr, file)) != ELF_SUCCESS) {
		return err;
	}

	pcb_t pcb;
	if ((err = initialize_process(&pcb)) != 0) {
		return err;
	}

	if ((err = initialize_memory(file, elf_hdr)) != 0) {
		return err;
	}

	tcb_t tcb;
	if ((err = initialize_thread(&pcb, &tcb)) != 0) {
		return err;
	}

	

/*@}*/
