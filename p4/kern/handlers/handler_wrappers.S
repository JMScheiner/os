#include <x86/idt.h>
#include <syscall_int.h>
#include <timer_defines.h>
#define KEY_IDT_ENTRY 0x21

#define NAME divide_error_handler
#define CAUSE IDT_DE
#include "handlers/handler.def"

#define NAME debug_handler
#define CAUSE IDT_DB
#include "handlers/handler.def"

#define NAME breakpoint_handler
#define CAUSE IDT_BP
#include "handlers/handler.def"

#define NAME overflow_handler
#define CAUSE IDT_OF
#include "handlers/handler.def"

#define NAME bound_range_exceeded_handler
#define CAUSE IDT_BR
#include "handlers/handler.def"

#define NAME invalid_opcode_handler
#define CAUSE IDT_UD
#include "handlers/handler.def"

#define NAME device_not_available_handler
#define CAUSE IDT_NM
#include "handlers/handler.def"

#define NAME double_fault_handler
#define CAUSE IDT_DF
#define ECODE_GENERATED 1
#include "handlers/handler.def"

#define NAME invalid_tss_handler
#define CAUSE IDT_TS
#define ECODE_GENERATED 1
#include "handlers/handler.def"

#define NAME segment_not_present_handler
#define CAUSE IDT_NP
#define ECODE_GENERATED 1
#include "handlers/handler.def"

#define NAME stack_segment_fault_handler
#define CAUSE IDT_SS
#define ECODE_GENERATED 1
#include "handlers/handler.def"

#define NAME general_protection_handler
#define CAUSE IDT_GP
#define ECODE_GENERATED 1
#include "handlers/handler.def"

#define NAME page_fault_handler
#define CAUSE IDT_PF
#define ECODE_GENERATED 1
#include "handlers/handler.def"

#define NAME alignment_check_handler
#define CAUSE IDT_AC
#define ECODE_GENERATED 1
#include "handlers/handler.def"

#define NAME machine_check_handler
#define CAUSE IDT_MC
#include "handlers/handler.def"

#define NAME syscall_handler
#define CAUSE SYSCALL_INT
#include "handlers/handler.def"

#define NAME fork_handler
#define CAUSE FORK_INT
#include "handlers/handler.def"

#define NAME exec_handler
#define CAUSE EXEC_INT
#include "handlers/handler.def"

#define NAME wait_handler
#define CAUSE WAIT_INT
#include "handlers/handler.def"

#define NAME deschedule_handler
#define CAUSE DESCHEDULE_INT
#include "handlers/handler.def"

#define NAME make_runnable_handler
#define CAUSE MAKE_RUNNABLE_INT
#include "handlers/handler.def"

#define NAME gettid_handler
#define CAUSE GETTID_INT
#include "handlers/handler.def"

#define NAME new_pages_handler
#define CAUSE NEW_PAGES_INT
#include "handlers/handler.def"

#define NAME remove_pages_handler
#define CAUSE REMOVE_PAGES_INT
#include "handlers/handler.def"

#define NAME sleep_handler
#define CAUSE SLEEP_INT
#include "handlers/handler.def"

#define NAME getchar_handler
#define CAUSE GETCHAR_INT
#include "handlers/handler.def"

#define NAME readline_handler
#define CAUSE READLINE_INT
#include "handlers/handler.def"

#define NAME print_handler
#define CAUSE PRINT_INT
#include "handlers/handler.def"

#define NAME set_term_color_handler
#define CAUSE SET_TERM_COLOR_INT
#include "handlers/handler.def"

#define NAME set_cursor_pos_handler
#define CAUSE SET_CURSOR_POS_INT
#include "handlers/handler.def"

#define NAME get_cursor_pos_handler
#define CAUSE GET_CURSOR_POS_INT
#include "handlers/handler.def"

#define NAME thread_fork_handler
#define CAUSE THREAD_FORK_INT
#include "handlers/handler.def"

#define NAME get_ticks_handler
#define CAUSE GET_TICKS_INT
#include "handlers/handler.def"

#define NAME yield_handler
#define CAUSE YIELD_INT
#include "handlers/handler.def"

#define NAME misbehave_handler
#define CAUSE MISBEHAVE_INT
#include "handlers/handler.def"

#define NAME halt_handler
#define CAUSE HALT_INT
#include "handlers/handler.def"

#define NAME ls_handler
#define CAUSE LS_INT
#include "handlers/handler.def"

#define NAME task_vanish_handler
#define CAUSE TASK_VANISH_INT
#include "handlers/handler.def"

#define NAME set_status_handler
#define CAUSE SET_STATUS_INT
#include "handlers/handler.def"

#define NAME vanish_handler
#define CAUSE VANISH_INT
#include "handlers/handler.def"

#define NAME swexn_handler
#define CAUSE SWEXN_INT
#include "handlers/handler.def"

#define IGNORE_THREAD_COUNT

#define NAME timer_handler
#define CAUSE TIMER_IDT_ENTRY
#include "handlers/handler.def"

#define NAME keyboard_handler
#define CAUSE KEY_IDT_ENTRY
#include "handlers/handler.def"

