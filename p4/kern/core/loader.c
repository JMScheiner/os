/**
 * @file loader.c
 *
 * @brief Functions for the loading
 * of user programs from binary 
 * files should be written in
 * this file. The function 
 * elf_load_helper() is provided
 * for your use.
 *
 * @author Tim Wilson
 * @author Justin Scheiner
 */

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
#include <ecodes.h>
#include <mm.h>
#include <region.h>

#include <process.h>
#include <thread.h>
#include <context_switch.h>
#include <mode_switch.h>
#include <eflags.h>
#include <assert.h>
#include <types.h>
#include <debug.h>
#include <mutex.h>
#include <macros.h>
#include <pagefault.h>

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
   assert(args > (char*)(USER_STACK_BASE - PAGE_SIZE));

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

/**
 * @brief Initialize a memory region for a new user with the appropriate
 * contents.
 *
 * @param file The executable file to intialize from.
 * @param offset The offset in the file to copy from.
 * @param len The number of bytes to copy, extra space in the region will
 *            be zeroed.
 * @param start The first address in memory to initialize
 * @param end The byte after the last address to initialize.
 */
static void initialize_region(const char *file, unsigned long offset, 
      unsigned long len, unsigned long start, unsigned long end) 
{
   getbytes(file, offset, len, (char *)start);
   memset((char *)start + len, 0, end - start - len);
}

/**
 * @brief Initialize memory for a user with the appropriate contents.
 *
 * @param file The executable to initialize from.
 * @param elf An elf header for the executable.
 * @param pcb The pcb of the process.
 *
 * @return ESUCCESS if initialization was successful, EFAIL otherwise.
 */
int initialize_memory(const char *file, simple_elf_t elf, pcb_t* pcb) 
{
   // Allocate text region. 
   if(allocate_region(
         (char*)elf.e_txtstart, (char *)elf.e_txtstart + elf.e_txtlen, 
         PTENT_RO | PTENT_USER, txt_fault, pcb) < 0) 
      goto fail_init_mem;
      
   // Allocate rodata region.
   if(allocate_region(
         (char*)elf.e_rodatstart, (char *)elf.e_rodatstart + elf.e_rodatlen, 
         PTENT_RO | PTENT_USER, rodata_fault, pcb) < 0) 
      goto fail_init_mem;
   
   // Allocate data region.
   if(allocate_region(
         (char*)elf.e_datstart, (char*)elf.e_datstart + elf.e_datlen,
         PTENT_RW | PTENT_USER,  dat_fault, pcb) < 0) 
      goto fail_init_mem;
   
   /* Allocate bss region. Despite what the spec says, this is not the
       very next byte after the dat section. It must be aligned... 
       It is not always 32 byte aligned as in mandelbrot, 
       Or 4 byte aligned as in cho_variant...
   
       Since we want to pass both of these tests. Our only recourse
        is to turn ZFOD off.  Frowny. Face. 
    */

   char *bss_start = (char*)(elf.e_datstart + elf.e_datlen);
   if(allocate_region(
         bss_start, bss_start + elf.e_bsslen, 
         PTENT_RW | PTENT_USER /*| PTENT_ZFOD*/, bss_fault, pcb) < 0) 
      goto fail_init_mem;
      
   // Allocate stack region (same for all processes).
   if(allocate_region(
      USER_STACK_BASE - PAGE_SIZE, USER_STACK_BASE, 
      PTENT_RW | PTENT_USER, stack_fault, pcb) < 0)
      goto fail_init_mem;
      
   initialize_region(file, elf.e_txtoff, elf.e_txtlen, 
      elf.e_txtstart, elf.e_rodatstart);
      
   initialize_region(file, elf.e_rodatoff, elf.e_rodatlen, 
      elf.e_rodatstart, elf.e_datstart);
   
   initialize_region(file, elf.e_datoff, elf.e_datlen, elf.e_datstart, 
      elf.e_datstart + elf.e_datlen + elf.e_bsslen);
         
   return ESUCCESS;

fail_init_mem:
   free_region_list(pcb);
   mm_free_user_space(pcb);
   return EFAIL;
}

/** 
* @brief Get an ELF header from an executable.
* 
* @param exec The name of the executable. 
* @param elf_hdr The returned elf header. 
* 
* @return A negative integer on error, zero on success. 
*/
int get_elf(char *exec, simple_elf_t *elf_hdr) {
   int err;
   if ((err = elf_check_header(exec)) != ELF_SUCCESS) {
      return err;
   }

   return elf_load_helper(elf_hdr, exec);
}




/** 
* @brief Jumps to the user process.
* 
* @param tcb The TCB of the starting thread. 
* @param exec The name of the executable. 
* @param stack The user stack to jump to.
* @param eip The entry point of the executable. 
*/
void switch_to_user(tcb_t *tcb, char *exec, void *stack, void *eip) {
   unsigned int user_eflags = get_user_eflags();
   debug_print("loader", "Running %s", exec);
   sim_reg_process(tcb->dir_p, exec);
   mode_switch(tcb->kstack, stack, user_eflags, eip);
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
   simple_elf_t elf_hdr;
   if ((err = get_elf(exec, &elf_hdr)) != ELF_SUCCESS)
      return err;
   
   pcb_t* pcb = initialize_process(TRUE);
   if(pcb == NULL) 
      return ENOMEM;
   
   set_cr3((int)pcb->dir_p);
   if ((err = initialize_memory(exec, elf_hdr, pcb)) != 0) {
      sfree(pcb->status, sizeof(status_t));
      free_process_resources(pcb, FALSE);
      return err;
   }
   
   tcb_t* tcb = initialize_thread(pcb);
   if(tcb == NULL)
   {
      /* Free all resources associated with the PCB. */
      sfree(pcb->status, sizeof(status_t));
      free_process_resources(pcb, FALSE);
      return ENOMEM;
   }

   void *stack = copy_to_stack(argc, argv, arg_len);

   quick_fake_unlock();
   scheduler_register(tcb);
   
   switch_to_user(tcb, exec, stack, (void *)elf_hdr.e_entry);

   // Never get here
   assert(FALSE);
   return 0;
}

/*@}*/
