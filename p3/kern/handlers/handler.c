#include "handler.h"

#include <timer_defines.h>
#include <idt.h>
#include <handlers/handler_wrappers.h>
#include <syscall_int.h>
#include <keyhelp.h>
#include <string.h>
#include <asm.h>
#include <stdio.h>

int handler_install()
{
	trap_gate_t tg;
	INSTALL_HANDLER(tg, asm_divide_error_handler, IDT_DE);
	INSTALL_HANDLER(tg, asm_debug_handler, IDT_DB);
	INSTALL_HANDLER(tg, asm_breakpoint_handler, IDT_BP);
	INSTALL_HANDLER(tg, asm_overflow_handler, IDT_OF);
	INSTALL_HANDLER(tg, asm_bound_range_exceeded_handler, IDT_BR);
	INSTALL_HANDLER(tg, asm_invalid_opcode_handler, IDT_UD);
	INSTALL_HANDLER(tg, asm_device_not_available_handler, IDT_NM);
	INSTALL_HANDLER(tg, asm_double_fault_handler, IDT_DF);
	INSTALL_HANDLER(tg, asm_invalid_tss_handler, IDT_TS);
	INSTALL_HANDLER(tg, asm_segment_not_present_handler, IDT_NP);
	INSTALL_HANDLER(tg, asm_stack_segment_fault_handler, IDT_SS);
	INSTALL_HANDLER(tg, asm_general_protection_handler, IDT_GP);
	INSTALL_HANDLER(tg, asm_page_fault_handler, IDT_PF);
	INSTALL_HANDLER(tg, asm_alignment_check_handler, IDT_AC);
	INSTALL_HANDLER(tg, asm_machine_check_handler, IDT_MC);
	INSTALL_USER_HANDLER(tg, asm_syscall_handler, SYSCALL_INT);
	INSTALL_USER_HANDLER(tg, asm_fork_handler, FORK_INT);
	INSTALL_USER_HANDLER(tg, asm_exec_handler, EXEC_INT);
	INSTALL_USER_HANDLER(tg, asm_wait_handler, WAIT_INT);
	INSTALL_USER_HANDLER(tg, asm_deschedule_handler, DESCHEDULE_INT);
	INSTALL_USER_HANDLER(tg, asm_make_runnable_handler, MAKE_RUNNABLE_INT);
	INSTALL_USER_HANDLER(tg, asm_gettid_handler, GETTID_INT);
	INSTALL_USER_HANDLER(tg, asm_new_pages_handler, NEW_PAGES_INT);
	INSTALL_USER_HANDLER(tg, asm_remove_pages_handler, REMOVE_PAGES_INT);
	INSTALL_USER_HANDLER(tg, asm_sleep_handler, SLEEP_INT);
	INSTALL_USER_HANDLER(tg, asm_getchar_handler, GETCHAR_INT);
	INSTALL_USER_HANDLER(tg, asm_readline_handler, READLINE_INT);
	INSTALL_USER_HANDLER(tg, asm_print_handler, PRINT_INT);
	INSTALL_USER_HANDLER(tg, asm_set_term_color_handler, SET_TERM_COLOR_INT);
	INSTALL_USER_HANDLER(tg, asm_set_cursor_pos_handler, SET_CURSOR_POS_INT);
	INSTALL_USER_HANDLER(tg, asm_get_cursor_pos_handler, GET_CURSOR_POS_INT);
	INSTALL_USER_HANDLER(tg, asm_thread_fork_handler, THREAD_FORK_INT);
	INSTALL_USER_HANDLER(tg, asm_get_ticks_handler, GET_TICKS_INT);
	INSTALL_USER_HANDLER(tg, asm_misbehave_handler, MISBEHAVE_INT);
	INSTALL_USER_HANDLER(tg, asm_halt_handler, HALT_INT);
	INSTALL_USER_HANDLER(tg, asm_ls_handler, LS_INT);
	INSTALL_USER_HANDLER(tg, asm_task_vanish_handler, TASK_VANISH_INT);
	INSTALL_USER_HANDLER(tg, asm_set_status_handler, SET_STATUS_INT);
	INSTALL_USER_HANDLER(tg, asm_vanish_handler, VANISH_INT);
	INSTALL_HANDLER(tg, asm_timer_handler, TIMER_IDT_ENTRY);
	INSTALL_HANDLER(tg, asm_keyboard_handler, KEY_IDT_ENTRY);


	return 0;
}
