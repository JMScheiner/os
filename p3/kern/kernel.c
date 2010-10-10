/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @author Fred Hacker (fhacker)
 *  @bug No known bugs.
 */

#include <common_kern.h>

/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */
#include <malloc.h>

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* memory includes. */
#include <lmm.h>                    /* lmm_remove_free() */
#include <mm.h>
#include <handler.h>

/* x86 specific includes */
#include <x86/seg.h>                /* install_user_segs() */
#include <x86/interrupt_defines.h>  /* interrupt_setup() */
#include <x86/asm.h>                /* enable_interrupts() */


/*
 * state for kernel memory allocation.
 */
extern lmm_t malloc_lmm;

/*
 * Info about system gathered by the boot loader
 */
extern struct multiboot_info boot_info;

/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
   /*
    * Tell the kernel memory allocator which memory it can't use.
    * It already knows not to touch kernel image.
    */

   /* Everything above 16M */
   lmm_remove_free( &malloc_lmm, (void*)USER_MEM_START, -8 - USER_MEM_START );

   /* Everything below 1M  */
   lmm_remove_free( &malloc_lmm, (void*)0, 0x100000 );

   /*
    * initialize the PIC so that IRQs and
    * exception handlers don't overlap in the IDT.
    */
   interrupt_setup();

   /*
    * When kernel_main() begins, interrupts are DISABLED.
    * You should delete this comment, and enable them --
    * when you are ready.
    */
   
   handler_install();
   mm_init();
	 process_table_init();
	 thread_table_init();
   lprintf( "Hello from a brand new kernel!" );
   int b = 0;
   int a = 3 / b;
   lprintf("%d", a);
   MAGIC_BREAK;

   while (1) {
      continue;
   }

   return 0;
}
