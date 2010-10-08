
#define NAME divide_error_handler
#include "handler.def"

#define NAME debug_handler
#include "handler.def"

#define NAME breakpoint_handler
#include "handler.def"

#define NAME overflow_handler
#include "handler.def"

#define NAME bound_range_exceeded_handler
#include "handler.def"

#define NAME invalid_opcode_handler
#include "handler.def"

#define NAME device_not_available_handler
#include "handler.def"

#define NAME double_fault_handler
#include "handler.def"

#define NAME invalid_tss_handler
#include "handler.def"

#define NAME segment_not_present_handler
#include "handler.def"

#define NAME stack_segment_fault_handler
#include "handler.def"

#define NAME general_protection_handler
#include "handler.def"

#define NAME page_fault_handler
#include "handler.def"

#define NAME alignment_check_handler
#include "handler.def"

#define NAME machine_check_handler
#include "handler.def"

