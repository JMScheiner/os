
#ifndef HANDLER_WRAPPERS_D3J21KDJ
#define HANDLER_WRAPPERS_D3J21KDJ

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

#endif

