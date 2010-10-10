#include <handlers/handler_wrappers.h>
#include <interrupt_defines.h>
#include <simics.h>
#include <asm.h>
#include <reg.h>

void divide_error_handler(regstate_t reg)
{
	lprintf("Ignoring divide_error ");
}

void debug_handler(regstate_t reg)
{
	lprintf("Ignoring debug ");
}

void breakpoint_handler(regstate_t reg)
{
	lprintf("Ignoring breakpoint ");
}

void overflow_handler(regstate_t reg)
{
	lprintf("Ignoring overflow ");
}

void bound_range_exceeded_handler(regstate_t reg)
{
	lprintf("Ignoring bound_range_exceeded ");
}

void invalid_opcode_handler(regstate_t reg)
{
	lprintf("Ignoring invalid_opcode ");
}

void device_not_available_handler(regstate_t reg)
{
	lprintf("Ignoring device_not_available ");
}

void double_fault_handler(regstate_t reg)
{
	lprintf("Ignoring double_fault ");
}

void invalid_tss_handler(regstate_t reg)
{
	lprintf("Ignoring invalid_tss ");
}

void segment_not_present_handler(regstate_t reg)
{
	lprintf("Ignoring segment_not_present ");
}

void stack_segment_fault_handler(regstate_t reg)
{
	lprintf("Ignoring stack_segment_fault ");
}

void general_protection_handler(regstate_t reg)
{
	lprintf("Ignoring general_protection ");
}

void alignment_check_handler(regstate_t reg)
{
	lprintf("Ignoring alignment_check ");
}

void machine_check_handler(regstate_t reg)
{
	lprintf("Ignoring machine_check ");
}

void syscall_handler(regstate_t reg)
{
	lprintf("Ignoring syscall ");
}

void fork_handler(regstate_t reg)
{
	lprintf("Ignoring fork ");
}

void exec_handler(regstate_t reg)
{
	lprintf("Ignoring exec ");
}

void wait_handler(regstate_t reg)
{
	lprintf("Ignoring wait ");
}

void deschedule_handler(regstate_t reg)
{
	lprintf("Ignoring deschedule ");
}

void make_runnable_handler(regstate_t reg)
{
	lprintf("Ignoring make_runnable ");
}

void new_pages_handler(regstate_t reg)
{
	lprintf("Ignoring new_pages ");
}

void remove_pages_handler(regstate_t reg)
{
	lprintf("Ignoring remove_pages ");
}

void sleep_handler(regstate_t reg)
{
	lprintf("Ignoring sleep ");
}

void getchar_handler(regstate_t reg)
{
	lprintf("Ignoring getchar ");
}

void readline_handler(regstate_t reg)
{
	lprintf("Ignoring readline ");
}

void print_handler(regstate_t reg)
{
	lprintf("Ignoring print ");
}

void set_term_color_handler(regstate_t reg)
{
	lprintf("Ignoring set_term_color ");
}

void set_cursor_pos_handler(regstate_t reg)
{
	lprintf("Ignoring set_cursor_pos ");
}

void get_cursor_pos_handler(regstate_t reg)
{
	lprintf("Ignoring get_cursor_pos ");
}

void thread_fork_handler(regstate_t reg)
{
	lprintf("Ignoring thread_fork ");
}

void get_ticks_handler(regstate_t reg)
{
	lprintf("Ignoring get_ticks ");
}

void misbehave_handler(regstate_t reg)
{
	lprintf("Ignoring misbehave ");
}

void halt_handler(regstate_t reg)
{
	lprintf("Ignoring halt ");
}

void ls_handler(regstate_t reg)
{
	lprintf("Ignoring ls ");
}

void task_vanish_handler(regstate_t reg)
{
	lprintf("Ignoring task_vanish ");
}

void set_status_handler(regstate_t reg)
{
	lprintf("Ignoring set_status ");
}

void vanish_handler(regstate_t reg)
{
	lprintf("Ignoring vanish ");
}

void timer_handler(regstate_t reg)
{
	lprintf("Ignoring timer ");
}

void key_handler(regstate_t reg)
{
	lprintf("Ignoring key ");
}

