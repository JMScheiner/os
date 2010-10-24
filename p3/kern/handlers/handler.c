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
	void* base = idt_base();

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_DE);
	INSTALL_HANDLER(tg, asm_divide_error_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_DB);
	INSTALL_HANDLER(tg, asm_debug_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_BP);
	INSTALL_HANDLER(tg, asm_breakpoint_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_OF);
	INSTALL_HANDLER(tg, asm_overflow_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_BR);
	INSTALL_HANDLER(tg, asm_bound_range_exceeded_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_UD);
	INSTALL_HANDLER(tg, asm_invalid_opcode_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_NM);
	INSTALL_HANDLER(tg, asm_device_not_available_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_DF);
	INSTALL_HANDLER(tg, asm_double_fault_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_TS);
	INSTALL_HANDLER(tg, asm_invalid_tss_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_NP);
	INSTALL_HANDLER(tg, asm_segment_not_present_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_SS);
	INSTALL_HANDLER(tg, asm_stack_segment_fault_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_GP);
	INSTALL_HANDLER(tg, asm_general_protection_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_PF);
	INSTALL_HANDLER(tg, asm_page_fault_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_AC);
	INSTALL_HANDLER(tg, asm_alignment_check_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * IDT_MC);
	INSTALL_HANDLER(tg, asm_machine_check_handler);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * SYSCALL_INT);
	INSTALL_HANDLER(tg, asm_syscall_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * FORK_INT);
	INSTALL_HANDLER(tg, asm_fork_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * EXEC_INT);
	INSTALL_HANDLER(tg, asm_exec_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * WAIT_INT);
	INSTALL_HANDLER(tg, asm_wait_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * DESCHEDULE_INT);
	INSTALL_HANDLER(tg, asm_deschedule_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * MAKE_RUNNABLE_INT);
	INSTALL_HANDLER(tg, asm_make_runnable_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * GETTID_INT);
	INSTALL_HANDLER(tg, asm_gettid_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * NEW_PAGES_INT);
	INSTALL_HANDLER(tg, asm_new_pages_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * REMOVE_PAGES_INT);
	INSTALL_HANDLER(tg, asm_remove_pages_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * SLEEP_INT);
	INSTALL_HANDLER(tg, asm_sleep_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * GETCHAR_INT);
	INSTALL_HANDLER(tg, asm_getchar_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * READLINE_INT);
	INSTALL_HANDLER(tg, asm_readline_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * PRINT_INT);
	INSTALL_HANDLER(tg, asm_print_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * SET_TERM_COLOR_INT);
	INSTALL_HANDLER(tg, asm_set_term_color_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * SET_CURSOR_POS_INT);
	INSTALL_HANDLER(tg, asm_set_cursor_pos_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * GET_CURSOR_POS_INT);
	INSTALL_HANDLER(tg, asm_get_cursor_pos_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * THREAD_FORK_INT);
	INSTALL_HANDLER(tg, asm_thread_fork_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * GET_TICKS_INT);
	INSTALL_HANDLER(tg, asm_get_ticks_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * YIELD_INT);
	INSTALL_HANDLER(tg, asm_yield_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * MISBEHAVE_INT);
	INSTALL_HANDLER(tg, asm_misbehave_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * HALT_INT);
	INSTALL_HANDLER(tg, asm_halt_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * LS_INT);
	INSTALL_HANDLER(tg, asm_ls_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * TASK_VANISH_INT);
	INSTALL_HANDLER(tg, asm_task_vanish_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * SET_STATUS_INT);
	INSTALL_HANDLER(tg, asm_set_status_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * VANISH_INT);
	INSTALL_HANDLER(tg, asm_vanish_handler);
	IDT_SET_DPL(tg, 0x3)

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * TIMER_IDT_ENTRY);
	INSTALL_HANDLER(tg, asm_timer_handler);
	IDT_MAKE_INTERRUPT(tg);

	tg = (trap_gate_t)(base + TRAP_GATE_SIZE * KEY_IDT_ENTRY);
	INSTALL_HANDLER(tg, asm_keyboard_handler);
	IDT_MAKE_INTERRUPT(tg);

	return 0;
}
