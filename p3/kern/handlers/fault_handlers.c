#include <handlers/handler_wrappers.h>
#include <interrupt_defines.h>
#include <simics.h>
#include <asm.h>
#include <reg.h>

void divide_error_handler(regstate_t reg)
{
	lprintf("Ignoring divide_error ");
	MAGIC_BREAK;
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
	lprintf("Ignoring overflow ");
	MAGIC_BREAK;
}

void bound_range_exceeded_handler(regstate_t reg)
{
	lprintf("Ignoring bound_range_exceeded ");
	MAGIC_BREAK;
}

void invalid_opcode_handler(regstate_t reg)
{
	lprintf("Ignoring invalid_opcode ");
	MAGIC_BREAK;
}

void device_not_available_handler(regstate_t reg)
{
	lprintf("Ignoring device_not_available ");
	MAGIC_BREAK;
}

void double_fault_handler(regstate_error_t reg)
{
	lprintf("Ignoring double_fault ");
	MAGIC_BREAK;
}

void invalid_tss_handler(regstate_error_t reg)
{
	lprintf("Ignoring invalid_tss ");
	MAGIC_BREAK;
}

void segment_not_present_handler(regstate_error_t reg)
{
	lprintf("Ignoring segment_not_present ");
	MAGIC_BREAK;
}

void stack_segment_fault_handler(regstate_error_t reg)
{
	lprintf("Ignoring stack_segment_fault ");
	MAGIC_BREAK;
}

void general_protection_handler(regstate_error_t reg)
{
	lprintf("Ignoring general_protection ");
	MAGIC_BREAK;
}

void alignment_check_handler(regstate_error_t reg)
{
	lprintf("Ignoring alignment_check ");
	MAGIC_BREAK;
}

void machine_check_handler(regstate_t reg)
{
	lprintf("Ignoring machine_check ");
	MAGIC_BREAK;
}

void syscall_handler(regstate_t reg)
{
	lprintf("Ignoring syscall ");
	MAGIC_BREAK;
}

void misbehave_handler(regstate_t reg)
{
	lprintf("Ignoring misbehave ");
	MAGIC_BREAK;
}

