
fwrap = open('handler_wrappers.S', 'w')
fwrapheader = open('handler_wrappers.h', 'w')
fhandler = open('handler.c', 'w')
ffaulthandler = open('fault_handlers.c', 'w')

handlers = [
   ['divide_error', 'IDT_DE', True, False], 
   ['debug', 'IDT_DB', True, False],
   ['breakpoint', 'IDT_BP', True, False],
   ['overflow', 'IDT_OF', True, False],
   ['bound_range_exceeded','IDT_BR', True, False],
   ['invalid_opcode', 'IDT_UD', True, False], 
   ['device_not_available', 'IDT_NM', True, False],
   ['double_fault', 'IDT_DF', True, False], 
   ['invalid_tss', 'IDT_TS', True, False],
   ['segment_not_present', 'IDT_NP', True, False],
   ['stack_segment_fault', 'IDT_SS', True, False],
   ['general_protection', 'IDT_GP', True, False],
   ['page_fault', 'IDT_PF', False, False],
   ['alignment_check', 'IDT_AC', True, False], 
   ['machine_check', 'IDT_MC', True, False],
   ['syscall', 'SYSCALL_INT', True, True], 
   ['fork', 'FORK_INT', False, True], 
   ['exec', 'EXEC_INT', False, True], 
   ['wait', 'WAIT_INT', False, True],
   ['deschedule', 'DESCHEDULE_INT', False, True], 
   ['make_runnable', 'MAKE_RUNNABLE_INT', False, True], 
   ['gettid', 'GETTID_INT', False, True], 
   ['new_pages', 'NEW_PAGES_INT', False, True], 
   ['remove_pages', 'REMOVE_PAGES_INT', False, True], 
   ['sleep', 'SLEEP_INT', False, True], 
   ['getchar', 'GETCHAR_INT', False, True], 
   ['readline', 'READLINE_INT', False, True],
   ['print', 'PRINT_INT', False, True], 
   ['set_term_color', 'SET_TERM_COLOR_INT', False, True], 
   ['set_cursor_pos', 'SET_CURSOR_POS_INT', False, True], 
   ['get_cursor_pos', 'GET_CURSOR_POS_INT', False, True], 
   ['thread_fork', 'THREAD_FORK_INT', False, True], 
   ['get_ticks', 'GET_TICKS_INT', False, True], 
   ['yield', 'YIELD_INT', False, True], 
   ['misbehave', 'MISBEHAVE_INT', True, True], 
   ['halt', 'HALT_INT', True, True], 
   ['ls', 'LS_INT', True, True], 
   ['task_vanish', 'TASK_VANISH_INT', False, True], 
   ['set_status', 'SET_STATUS_INT', False, True],
   ['vanish', 'VANISH_INT', False, True],
   ['timer', 'TIMER_IDT_ENTRY', False, False],
   ['keyboard', 'KEY_IDT_ENTRY', False, False]
]


fwrapheader.write('#ifndef _HANDLER_WRAPPER_H_\n')
fwrapheader.write('#define _HANDLER_WRAPPER_H_\n\n')

fhandler.write('#include \"handler.h\"\n\n')

fhandler.write('#include <timer_defines.h>\n')
fhandler.write('#include <idt.h>\n')
fhandler.write('#include <handlers/handler_wrappers.h>\n')
fhandler.write('#include <syscall_int.h>\n')
fhandler.write('#include <keyhelp.h>\n')
fhandler.write('#include <string.h>\n')
fhandler.write('#include <asm.h>\n')
fhandler.write('#include <stdio.h>\n\n')
fhandler.write('int handler_install()\n{\n\ttrap_gate_t tg;\n')

ffaulthandler.write('#include <handlers/handler_wrappers.h>\n')
ffaulthandler.write('#include <interrupt_defines.h>\n')
ffaulthandler.write('#include <simics.h>\n')
ffaulthandler.write('#include <asm.h>\n')
ffaulthandler.write('#include <reg.h>\n\n')

for l in handlers: 
   fwrap.write('#define NAME ' + l[0] + '_handler\n')
   fwrap.write('#include \"handlers/handler.def\"\n\n')
   fwrapheader.write('void asm_' + l[0] + '_handler(void);\n\n')
   
   if l[3]: 
      fhandler.write('\tINSTALL_USER_HANDLER(tg, asm_' +   
         l[0] + '_handler, ' + l[1] + ');\n')
   else:
      fhandler.write('\tINSTALL_HANDLER(tg, asm_' +   
         l[0] + '_handler, ' + l[1] + ');\n')

   if l[2]:
      ffaulthandler.write('void ' + l[0] + '_handler(regstate_t reg)\n{\n')
      ffaulthandler.write('\tlprintf(\"Ignoring ' + l[0] + ' \");\n')
      ffaulthandler.write('\tMAGIC_BREAK;\n')
      ffaulthandler.write('}\n\n')


fwrapheader.write('#endif //_HANDLER_WRAPPER_H_\n')
fhandler.write('\n\n\treturn 0;\n}\n')


