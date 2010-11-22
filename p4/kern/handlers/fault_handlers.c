/** 
* @file fault_handlers.c
* @brief Remaining fault handlers. 
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-11-12
*/
#include <handlers/handler_wrappers.h>
#include <interrupt_defines.h>
#include <simics.h>
#include <asm.h>
#include <reg.h>
#include <lifecycle.h>
#include <stdio.h>
#include <assert.h>
#include <asm_helper.h>
#include <ureg.h>

#define ERRBUF_SIZE 0x100

/** 
* @brief Kills a thread on divide by zero. 
* 
* @param reg The register state on entry to the handler.
*/
void divide_error_handler(ureg_t* reg)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Divide by zero, %%eip = 0x%d", reg->eip);
   thread_kill(errbuf);
}

/** 
* @brief Ignores debug mode.
* 
* @param reg The register state on entry to the handler.
*/
void debug_handler(ureg_t* reg)
{
   lprintf("Ignoring debug ");
}

/** 
* @brief Ignores breakpoints.
* 
* @param reg The register state on entry to the handler.
*/
void breakpoint_handler(ureg_t* reg)
{
   lprintf("Ignoring breakpoint ");
}

/** 
* @brief Ignores overflows.
* 
* @param reg The register state on entry to the handler.
*/
void overflow_handler(ureg_t* reg)
{
   /* Explicitly ignore overflows, since we have no signal mechanism. */
}

/** 
* @brief Ignores bound range exceeded.
* 
* @param reg The register state on entry to the handler.
*/
void bound_range_exceeded_handler(ureg_t* reg)
{
   /* Explicitly ignore bound range exceeded, 
    * since we have no signal mechanism. */
}

/** 
* @brief Kills threads on invalid opcodes. 
* 
* @param reg The register state on entry to the handler.
*/
void invalid_opcode_handler(ureg_t* reg)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Invalid instruction, %%eip = 0x%d", reg->eip);
   thread_kill(errbuf);
}

/** 
* @brief Kills threads if they try to do floating point math. 
* 
* @param reg The register state on entry to the handler.
*/
void device_not_available_handler(ureg_t* reg)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Device not available exception at %%eip = 0x%d", reg->eip);
   thread_kill(errbuf);
}

/** 
* @brief We don't double fault. Ever. 
* 
* @param reg The register state on entry to the handler.
*/
void double_fault_handler(ureg_t* reg)
{
   /* This should never happen.*/
   assert(0);
}

/** 
* @brief We only ever use one TSS. And it is valid. 
*  This never happens.
* 
* @param reg The register state on entry to the handler.
*/
void invalid_tss_handler(ureg_t* reg)
{
   /* This should never happen.*/
   assert(0); 
}

/** 
* @brief Our segments cover all of memory. This doesn't happen.
* 
* @param reg The register state on entry to the handler.
*/
void segment_not_present_handler(ureg_t* reg)
{
   /* This should never happen.*/
   assert(0); 
}

/** 
* @brief Our segments cover all of memory. This doesn't happen.
* 
* @param reg The register state on entry to the handler.
*/
void stack_segment_fault_handler(ureg_t* reg)
{
   /* This should never happen.*/
   assert(0); 
}

/** 
* @brief We don't general protection fault. (Hopefully!) 
* 
* @param reg The register state on entry to the handler.
*/
void general_protection_handler(ureg_t* reg)
{
   /* This should never happen.*/
   assert(0); 
}

/** 
* @brief We don't do alignment checking.
* 
* @param reg The register state on entry to the handler.
*/
void alignment_check_handler(ureg_t* reg)
{
   assert(0); 
}

/** 
* @brief This is catastrophic. halt.
* 
* @param reg The register state on entry to the handler.
*/
void machine_check_handler(ureg_t* reg)
{
   /* Something disastrous happened. */
   halt();
}

/** 
* @brief Ignored.
* 
* @param reg The register state on entry to the handler.
*/
void syscall_handler(ureg_t* reg)
{
   /* Doesn't do anything. */
}

/** 
* @brief Ignored
* 
* @param reg The register state on entry to the handler.
*/
void misbehave_handler(ureg_t* reg)
{
   /* Doesn't do anything. */
}

