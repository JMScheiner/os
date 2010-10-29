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
#include <assert.h>
#include <string.h>
#include <timer.h>
#include <console.h>
#include <scheduler.h>
#include <keyboard.h>

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

#include <malloc_wrappers.h>
#include <process.h>
#include <thread.h>
#include <loader.h>
#include <types.h>
#include <global_thread.h>
#include <asm_helper.h>
#include <lifecycle.h>

/**
 * @brief The first program to load.
 */
#define INIT_PROGRAM "coolness"

/*
 * state for kernel memory allocation.
 */
extern lmm_t malloc_lmm;

extern boolean_t locks_enabled;

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

   /* Give the "global process" a pcb and tcb. */
   global_pcb()->pid = -1;
   global_pcb()->ppid = -1;
   global_pcb()->thread_count = 1;
   global_pcb()->regions = NULL;
   
   /* It is fine to begin execution wherever we happen to be now
    *  since we are guaranteed to launch a thread before executing on
    *  this stack, and this thread of execution should never start up
    *  again.
    */
   global_tcb()->kstack = get_esp();
   global_tcb()->esp = global_tcb()->kstack;
   global_tcb()->tid = -1;
   global_tcb()->pcb = global_pcb();

	alloc_init();
   timer_init();
   keyboard_init();
   scheduler_init();
   init_process_table();
   init_thread_table();
   
   handler_install();
   clear_console();
   mm_init();
   arrange_global_context();

   locks_enabled = TRUE;
   enable_interrupts();

   while(1) { } 
   
   assert(0);
   return 0;
}
