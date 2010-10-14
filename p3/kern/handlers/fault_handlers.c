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

void double_fault_handler(regstate_t reg)
{
	lprintf("Ignoring double_fault ");
	MAGIC_BREAK;
}

void invalid_tss_handler(regstate_t reg)
{
	lprintf("Ignoring invalid_tss ");
	MAGIC_BREAK;
}

void segment_not_present_handler(regstate_t reg)
{
	lprintf("Ignoring segment_not_present ");
	MAGIC_BREAK;
}

void stack_segment_fault_handler(regstate_t reg)
{
	lprintf("Ignoring stack_segment_fault ");
	MAGIC_BREAK;
}

void general_protection_handler(regstate_t reg)
{
	lprintf("Ignoring general_protection ");
	MAGIC_BREAK;
}

void alignment_check_handler(regstate_t reg)
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

void fork_handler(regstate_t reg)
{
	lprintf("Ignoring fork ");
	MAGIC_BREAK;
}

void wait_handler(regstate_t reg)
{
	lprintf("Ignoring wait ");
	MAGIC_BREAK;
}

void deschedule_handler(regstate_t reg)
{
	lprintf("Ignoring deschedule ");
	MAGIC_BREAK;
}

void make_runnable_handler(regstate_t reg)
{
	lprintf("Ignoring make_runnable ");
	MAGIC_BREAK;
}

void new_pages_handler(regstate_t reg)
{
	lprintf("Ignoring new_pages ");
	MAGIC_BREAK;
}

void remove_pages_handler(regstate_t reg)
{
	lprintf("Ignoring remove_pages ");
	MAGIC_BREAK;
}

void sleep_handler(regstate_t reg)
{
	lprintf("Ignoring sleep ");
	MAGIC_BREAK;
}

void getchar_handler(regstate_t reg)
{
	lprintf("Ignoring getchar ");
	MAGIC_BREAK;
}

void readline_handler(regstate_t reg)
{
	lprintf("Ignoring readline ");
	MAGIC_BREAK;
}

void print_handler(regstate_t reg)
{
	lprintf("Ignoring print ");
	MAGIC_BREAK;
}

void set_term_color_handler(regstate_t reg)
{
	lprintf("Ignoring set_term_color ");
	MAGIC_BREAK;
}

void set_cursor_pos_handler(regstate_t reg)
{
	lprintf("Ignoring set_cursor_pos ");
	MAGIC_BREAK;
}

void get_cursor_pos_handler(regstate_t reg)
{
	lprintf("Ignoring get_cursor_pos ");
	MAGIC_BREAK;
}

void thread_fork_handler(regstate_t reg)
{
	lprintf("Ignoring thread_fork ");
	MAGIC_BREAK;
}

void get_ticks_handler(regstate_t reg)
{
	lprintf("Ignoring get_ticks ");
	MAGIC_BREAK;
}

void misbehave_handler(regstate_t reg)
{
	lprintf("Ignoring misbehave ");
	MAGIC_BREAK;
}

void halt_handler(regstate_t reg)
{
	lprintf("Ignoring halt ");
	MAGIC_BREAK;
}

void ls_handler(regstate_t reg)
{
	lprintf("Ignoring ls ");
	MAGIC_BREAK;
}

void task_vanish_handler(regstate_t reg)
{
	lprintf("Ignoring task_vanish ");
	MAGIC_BREAK;
}

void set_status_handler(regstate_t reg)
{
	lprintf("Ignoring set_status ");
	MAGIC_BREAK;
}

void vanish_handler(regstate_t reg)
{
	lprintf("Ignoring vanish ");
	MAGIC_BREAK;
}

