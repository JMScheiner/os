# 
# @file handler_files.py
# @brief I am not proud of this. 
# @author Justin Scheiner
# @date 2010-10-21
#

fwrap = open('handler_wrappers.S', 'w')
fwrapheader = open('handler_wrappers.h', 'w')
fhandler = open('handler.c', 'w')
ffaulthandler = open('fault_handlers.c', 'w')

name = 0
idt_entry = 1
implemented = 2
syscall = 3
ecode = 4

# [name, IDT entry, Implemented, DPL 3, Error code]
handlers = [
   ['divide_error',        'IDT_DE',            True, False, False], 
   ['debug',               'IDT_DB',            True, False, False],
   ['breakpoint',          'IDT_BP',            True, False, False],
   ['overflow',            'IDT_OF',            True, False, False],
   ['bound_range_exceeded','IDT_BR',            True, False, False],
   ['invalid_opcode',      'IDT_UD',            True, False, False], 
   ['device_not_available','IDT_NM',            True, False, False],
   ['double_fault',        'IDT_DF',            True, False, True], 
   ['invalid_tss',         'IDT_TS',            True, False, True],
   ['segment_not_present', 'IDT_NP',            True, False, True],
   ['stack_segment_fault', 'IDT_SS',            True, False, True],
   ['general_protection',  'IDT_GP',            True, False, True],
   ['page_fault',          'IDT_PF',            False, False, True],
   ['alignment_check',     'IDT_AC',            True, False, True], 
   ['machine_check',       'IDT_MC',            True, False, False],
   ['syscall',             'SYSCALL_INT',       True, True, False], 
   ['fork',                'FORK_INT',          False, True, False], 
   ['exec',                'EXEC_INT',          False, True, False], 
   ['wait',                'WAIT_INT',          False, True, False],
   ['deschedule',          'DESCHEDULE_INT',    False, True, False], 
   ['make_runnable',       'MAKE_RUNNABLE_INT', False, True, False], 
   ['gettid',              'GETTID_INT',        False, True, False], 
   ['new_pages',           'NEW_PAGES_INT',     False, True, False], 
   ['remove_pages',        'REMOVE_PAGES_INT',  False, True, False], 
   ['sleep',               'SLEEP_INT',         False, True, False], 
   ['getchar',             'GETCHAR_INT',       False, True, False], 
   ['readline',            'READLINE_INT',      False, True, False],
   ['print',               'PRINT_INT',         False, True, False], 
   ['set_term_color',      'SET_TERM_COLOR_INT',False, True, False], 
   ['set_cursor_pos',      'SET_CURSOR_POS_INT',False, True, False], 
   ['get_cursor_pos',      'GET_CURSOR_POS_INT',False, True, False], 
   ['thread_fork',         'THREAD_FORK_INT',   False, True, False], 
   ['get_ticks',           'GET_TICKS_INT',     False, True, False], 
   ['yield',               'YIELD_INT',         False, True, False], 
   ['misbehave',           'MISBEHAVE_INT',     True, True, False], 
   ['halt',                'HALT_INT',          True, True, False], 
   ['ls',                  'LS_INT',            True, True, False], 
   ['task_vanish',         'TASK_VANISH_INT',   False, True, False], 
   ['set_status',          'SET_STATUS_INT',    False, True, False],
   ['vanish',              'VANISH_INT',        False, True, False],
   ['timer',               'TIMER_IDT_ENTRY',   False, False, False],
   ['keyboard',            'KEY_IDT_ENTRY',     False, False, False]
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

for handler in handlers: 
   fwrap.write('#define NAME ' + handler[name] + '_handler\n')
   if handler[ecode]:
      fwrap.write('#include \"handlers/ehandler.def\"\n\n')
   else:
      fwrap.write('#include \"handlers/handler.def\"\n\n')
   
   fwrapheader.write('void asm_' + handler[name] + '_handler(void);\n\n')
   
   if handler[syscall]: 
      fhandler.write('\tINSTALL_USER_HANDLER(tg, asm_' +   
         handler[name] + '_handler, ' + handler[idt_entry] + ');\n')
   else:
      fhandler.write('\tINSTALL_HANDLER(tg, asm_' +   
         handler[name] + '_handler, ' + handler[idt_entry] + ');\n')

   if handler[implemented]:
      if handler[ecode]:
         ffaulthandler.write('void ' + handler[name] + '_handler(regstate_error_t reg)\n{\n')
      else:
         ffaulthandler.write('void ' + handler[name] + '_handler(regstate_t reg)\n{\n')
      
      ffaulthandler.write('\tlprintf(\"Ignoring ' + handler[name] + ' \");\n')
      ffaulthandler.write('\tMAGIC_BREAK;\n')
      ffaulthandler.write('}\n\n')


fwrapheader.write('#endif //_HANDLER_WRAPPER_H_\n')
fhandler.write('\n\n\treturn 0;\n}\n')


