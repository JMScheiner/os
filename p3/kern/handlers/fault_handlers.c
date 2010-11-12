#include <handlers/handler_wrappers.h>
#include <interrupt_defines.h>
#include <simics.h>
#include <asm.h>
#include <reg.h>
#include <lifecycle.h>
#include <stdio.h>
#include <assert.h>
#include <asm_helper.h>

#define ERRBUF_SIZE 0x100

void divide_error_handler(regstate_t reg)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Divide by zero, %%eip = 0x%ld", reg.eip);
   thread_kill(errbuf);
}

void debug_handler(regstate_t reg)
{
   lprintf("Ignoring debug ");
   MAGIC_BREAK;
}

void breakpoint_handler(regstate_t reg)
{
   lprintf("Ignoring breakpoint ");
   MAGIC_BREAK;
}

void overflow_handler(regstate_t reg)
{
   /* Explicitly ignore overflows, since we have no signal mechanism. */
}

void bound_range_exceeded_handler(regstate_t reg)
{
   /* Explicitly ignore bound range exceeded, 
    * since we have no signal mechanism. */
}

void invalid_opcode_handler(regstate_t reg)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Invalid instruction, %%eip = 0x%ld", reg.eip);
   thread_kill(errbuf);
}

void device_not_available_handler(regstate_t reg)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Device not available exception at %%eip = 0x%ld", reg.eip);
   thread_kill(errbuf);
}

void double_fault_handler(regstate_error_t reg)
{
   /* This never happens.*/
   MAGIC_BREAK;
   assert(0);
}

void invalid_tss_handler(regstate_error_t reg)
{
   /* This never happens.*/
   MAGIC_BREAK;
   assert(0); 
}

void segment_not_present_handler(regstate_error_t reg)
{
   /* This never happens.*/
   MAGIC_BREAK;
   assert(0); 
}

void stack_segment_fault_handler(regstate_error_t reg)
{
   /* This never happens.*/
   MAGIC_BREAK;
   assert(0); 
}

void general_protection_handler(regstate_error_t reg)
{
   /* This never happens.*/
   MAGIC_BREAK;
   assert(0); 
}

void alignment_check_handler(regstate_error_t reg)
{
   /* We don't do alignment checking. */
   MAGIC_BREAK;
   assert(0); 
}

void machine_check_handler(regstate_t reg)
{
   /* Something disastrous happened. */
   MAGIC_BREAK;
   halt();
}

void syscall_handler(regstate_t reg)
{
   /* Doesn't do anything. */
}

void misbehave_handler(regstate_t reg)
{
   /* Doesn't do anything. */
}

