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

# The name of the handler. 
name = 0

# The name of it's IDT entry (the constant).
idt_entry = 1

# Whether or not we have implemented it yet. 
not_implemented = 2

# Whether it is a syscall or not (can be invoked by the user).
syscall = 3

# Whether it generates an error code or not. 
ecode = 4

# Whether it is an interrupt gate or a trap gate.
interrupt = 5

# [name, IDT entry, Not Implemented, DPL 3, Error code, Interrupt]
handlers = [
   ['divide_error',        'IDT_DE',            False, False, False, False], 
   ['debug',               'IDT_DB',            False, False, False, False],
   ['breakpoint',          'IDT_BP',            False, False, False, False],
   ['overflow',            'IDT_OF',            False, False, False, False],
   ['bound_range_exceeded','IDT_BR',            False, False, False, False],
   ['invalid_opcode',      'IDT_UD',            False, False, False, False], 
   ['device_not_available','IDT_NM',            False, False, False, False],
   ['double_fault',        'IDT_DF',            False, False, True, False], 
   ['invalid_tss',         'IDT_TS',            False, False, True, False],
   ['segment_not_present', 'IDT_NP',            False, False, True, False],
   ['stack_segment_fault', 'IDT_SS',            False, False, True, False],
   ['general_protection',  'IDT_GP',            False, False, True, False],
   ['page_fault',          'IDT_PF',            False, False, True, False],
   ['alignment_check',     'IDT_AC',            False, False, True, False], 
   k'machine_check',       'IDT_MC',            False, False, False, False],
   ['syscall',             'SYSCALL_INT',       False, True, False, False], 
   ['fork',                'FORK_INT',          False, True, False, False], 
   ['exec',                'EXEC_INT',          False, True, False, False], 
   ['wait',                'WAIT_INT',          False, True, False, False],
   ['deschedule',          'DESCHEDULE_INT',    False, True, False, False], 
   ['make_runnable',       'MAKE_RUNNABLE_INT', False, True, False, False], 
   ['gettid',              'GETTID_INT',        False, True, False, False], 
   ['new_pages',           'NEW_PAGES_INT',     False, True, False, False], 
   ['remove_pages',        'REMOVE_PAGES_INT',  False, True, False, False], 
   ['sleep',               'SLEEP_INT',         False, True, False, False], 
   ['getchar',             'GETCHAR_INT',       False, True, False, False], 
   ['readline',            'READLINE_INT',      False, True, False, False],
   ['print',               'PRINT_INT',         False, True, False, False], 
   ['set_term_color',      'SET_TERM_COLOR_INT',False, True, False, False], 
   ['set_cursor_pos',      'SET_CURSOR_POS_INT',False, True, False, False], 
   ['get_cursor_pos',      'GET_CURSOR_POS_INT',False, True, False, False], 
   ['thread_fork',         'THREAD_FORK_INT',   False, True, False, False], 
   ['get_ticks',           'GET_TICKS_INT',     False, True, False, False], 
   ['yield',               'YIELD_INT',         False, True, False, False], 
   ['misbehave',           'MISBEHAVE_INT',     False, True, False, False], 
   ['halt',                'HALT_INT',          False, True, False, False], 
   ['ls',                  'LS_INT',            False, True, False, False], 
   ['task_vanish',         'TASK_VANISH_INT',   False, True, False, False], 
   ['set_status',          'SET_STATUS_INT',    False, True, False, False],
   ['vanish',              'VANISH_INT',        False, True, False, False],
   ['timer',               'TIMER_IDT_ENTRY',   False, False, False, True],
   ['keyboard',            'KEY_IDT_ENTRY',     False, False, False, True]
]



# Guard for handler_wrappers.h
fwrapheader.write('#ifndef _HANDLER_WRAPPERS_H_\n')
fwrapheader.write('#define _HANDLER_WRAPPERS_H_\n\n')

# Includes for handler.c
fhandler.write('#include \"handler.h\"\n\n')
fhandler.write('#include <timer_defines.h>\n')
fhandler.write('#include <idt.h>\n')
fhandler.write('#include <handlers/handler_wrappers.h>\n')
fhandler.write('#include <syscall_int.h>\n')
fhandler.write('#include <keyhelp.h>\n')
fhandler.write('#include <string.h>\n')
fhandler.write('#include <asm.h>\n')
fhandler.write('#include <stdio.h>\n\n')

# Function definition plus the one local variable.
fhandler.write('int handler_install()\n{\n')
fhandler.write('\ttrap_gate_t tg;\n')
fhandler.write('\tvoid* base = idt_base();\n\n')

# Includes for fault_handlers.c 
ffaulthandler.write('#include <handlers/handler_wrappers.h>\n')
ffaulthandler.write('#include <interrupt_defines.h>\n')
ffaulthandler.write('#include <simics.h>\n')
ffaulthandler.write('#include <asm.h>\n')
ffaulthandler.write('#include <reg.h>\n\n')

for handler in handlers: 
   
   # Write the two lines that expand into the assembly definition of 
   # the handlers wraper. 
   fwrap.write('#define NAME ' + handler[name] + '_handler\n')
   if handler[ecode]:
      fwrap.write('#include \"handlers/ehandler.def\"\n\n')
   else:
      fwrap.write('#include \"handlers/handler.def\"\n\n')
   
   # Write the handler wrappers definition into the header. 
   fwrapheader.write('void asm_' + handler[name] + '_handler(void);\n\n')
   
   # Write the code to install the handler into the IDT 
   fhandler.write('\ttg = (trap_gate_t)(base + TRAP_GATE_SIZE * ' + handler[idt_entry] + ');\n')
   fhandler.write('\tINSTALL_HANDLER(tg, asm_' + handler[name] + '_handler);\n')
   if handler[syscall]: 
      fhandler.write('\tIDT_SET_DPL(tg, 0x3)\n')
   if handler[interrupt]:
      fhandler.write('\tIDT_MAKE_INTERRUPT(tg);\n')
   fhandler.write('\n');
   
   
   # If we haven't implemented this handler yet, fill in an arbitrary definition in 
   #  fault_handlers.c (A simics printout + a magic break.)
   if handler[not_implemented]:
      if handler[ecode]:
         ffaulthandler.write('void ' + handler[name] + '_handler(regstate_error_t reg)\n{\n')
      else:
         ffaulthandler.write('void ' + handler[name] + '_handler(regstate_t reg)\n{\n')
      
      ffaulthandler.write('\tlprintf(\"Ignoring ' + handler[name] + ' \");\n')
      ffaulthandler.write('\tMAGIC_BREAK;\n')
      ffaulthandler.write('}\n\n')

# Close the guard for the wrapper header file. 
fwrapheader.write('#endif //_HANDLER_WRAPPER_H_\n')

# Write the end of the function that installs all of the handlers. 
fhandler.write('\treturn 0;\n}\n')

