
fwrap = open('handler_wrappers.S', 'w')
fwrapheader = open('handler_wrappers.h', 'w')
fhandler = open('handler.c', 'w');
ffaulthandler = open('fault_handlers.c', 'w')


handlers = [
   ['divide_error', 'IDT_DE', True], 
   ['debug', 'IDT_DB', True],
   ['breakpoint', 'IDT_BP', True],
   ['overflow', 'IDT_OF', True],
   ['bound_range_exceeded','IDT_BR', True],
   ['invalid_opcode', 'IDT_UD', True], 
   ['device_not_available', 'IDT_NM', True],
   ['double_fault', 'IDT_DF', True], 
   ['invalid_tss', 'IDT_TS', True],
   ['segment_not_present', 'IDT_NP', True],
   ['stack_segment_fault', 'IDT_SS', True],
   ['general_protection', 'IDT_GP', True],
   ['page_fault', 'IDT_PF', True],
   ['alignment_check', 'IDT_AC', True], 
   ['machine_check', 'IDT_MC', True],
   ['syscall', 'SYSCALL_INT', True], 
   ['fork', 'FORK_INT', True], 
   ['exec', 'EXEC_INT', True], 
   ['wait', 'WAIT_INT', True],
   ['deschedule', 'DESCHEDULE_INT', True], 
   ['make_runnable', 'MAKE_RUNNABLE_INT', True], 
   ['gettid', 'GETTID_INT', False], 
   ['new_pages', 'NEW_PAGES_INT', True], 
   ['remove_pages', 'REMOVE_PAGES_INT', True], 
   ['sleep', 'SLEEP_INT', True], 
   ['getchar', 'GETCHAR_INT', True], 
   ['readline', 'READLINE_INT', True],
   ['print', 'PRINT_INT', True], 
   ['set_term_color', 'SET_TERM_COLOR_INT', True], 
   ['set_cursor_pos', 'SET_CURSOR_POS_INT', True], 
   ['get_cursor_pos', 'GET_CURSOR_POS_INT', True], 
   ['thread_fork', 'THREAD_FORK_INT', True], 
   ['get_ticks', 'GET_TICKS_INT', True], 
   ['misbehave', 'MISBEHAVE_INT', True], 
   ['halt', 'HALT_INT', True], 
   ['ls', 'LS_INT', True], 
   ['task_vanish', 'TASK_VANISH_INT', True], 
   ['set_status', 'SET_STATUS_INT', True],
   ['vanish', 'VANISH_INT', True]
]


fwrapheader.write('#ifndef _HANDLER_WRAPPER_H_\n')
fwrapheader.write('#define _HANDLER_WRAPPER_H_\n\n')

fhandler.write('#include \"handler.h\"\n\n');

fhandler.write('#include <timer_defines.h>\n');
fhandler.write('#include <idt.h>\n');
fhandler.write('#include <handlers/handler_wrappers.h>\n');
fhandler.write('#include <syscall_int.h>\n');
fhandler.write('#include <keyhelp.h>\n');
fhandler.write('#include <string.h>\n');
fhandler.write('#include <asm.h>\n');
fhandler.write('#include <stdio.h>\n\n');
fhandler.write('int handler_install()\n{\n\ttrap_gate_t tg;\n');

ffaulthandler.write('#include <handlers/handler_wrappers.h>\n');
ffaulthandler.write('#include <interrupt_defines.h>\n');
ffaulthandler.write('#include <asm.h>\n');
ffaulthandler.write('#include <reg.h>\n\n');

for l in handlers: 
   fwrap.write('#define NAME ' + l[0] + '_handler\n')
   fwrap.write('#include \"handlers/handler.def\"\n\n')
   fwrapheader.write('void asm_' + l[0] + '_handler(void);\n\n');
   fhandler.write('\tINSTALL_HANDLER(tg, asm_' + l[0] + '_handler, ' + l[1] + ');\n');
   if l[2]:
      ffaulthandler.write('void ' + l[0] + '_handler(regstate_t reg)\n{\n\n}\n\n')


fwrapheader.write('#endif //_HANDLER_WRAPPER_H_\n')
fhandler.write('\n\n\treturn 0;\n}\n)');
