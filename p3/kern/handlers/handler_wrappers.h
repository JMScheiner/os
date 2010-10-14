#ifndef _HANDLER_WRAPPER_H_
#define _HANDLER_WRAPPER_H_

void asm_divide_error_handler(void);

void asm_debug_handler(void);

void asm_breakpoint_handler(void);

void asm_overflow_handler(void);

void asm_bound_range_exceeded_handler(void);

void asm_invalid_opcode_handler(void);

void asm_device_not_available_handler(void);

void asm_double_fault_handler(void);

void asm_invalid_tss_handler(void);

void asm_segment_not_present_handler(void);

void asm_stack_segment_fault_handler(void);

void asm_general_protection_handler(void);

void asm_page_fault_handler(void);

void asm_alignment_check_handler(void);

void asm_machine_check_handler(void);

void asm_syscall_handler(void);

void asm_fork_handler(void);

void asm_exec_handler(void);

void asm_wait_handler(void);

void asm_deschedule_handler(void);

void asm_make_runnable_handler(void);

void asm_gettid_handler(void);

void asm_new_pages_handler(void);

void asm_remove_pages_handler(void);

void asm_sleep_handler(void);

void asm_getchar_handler(void);

void asm_readline_handler(void);

void asm_print_handler(void);

void asm_set_term_color_handler(void);

void asm_set_cursor_pos_handler(void);

void asm_get_cursor_pos_handler(void);

void asm_thread_fork_handler(void);

void asm_get_ticks_handler(void);

void asm_misbehave_handler(void);

void asm_halt_handler(void);

void asm_ls_handler(void);

void asm_task_vanish_handler(void);

void asm_set_status_handler(void);

void asm_vanish_handler(void);

void asm_timer_handler(void);

void asm_keyboard_handler(void);

#endif //_HANDLER_WRAPPER_H_
