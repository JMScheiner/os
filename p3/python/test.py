from simics import *

cpu = current_processor()
eval_sym = SIM_get_class_interface("symtable", "symtable").eval_sym

#### util functions ####

def ptr_str(typed_val): 
   (type, val) = typed_val 
   return "((%s)0x%x)" % (type, val)

def eval_expr(cpu, expr): 
   return eval_sym(cpu, expr, [], 'v')

#### TCB manipulation ####

def next_tcb(tcb):
   return eval_expr(cpu, ptr_str(tcb) + '->scheduler_node.next')

def next_pcb(pcb):
   return eval_expr(cpu, ptr_str(pcb) + '->global_node.next')

def tid_of(tcb):
   type, tid = eval_expr(cpu, ptr_str(tcb) + '->tid')
   return tid

def get_runnable():
   return eval_expr(cpu, 'runnable')

def get_global():
   return eval_expr(cpu, '&_global_pcb')

def regstate_of(tcb):
   regstate_p = eval_expr(cpu, '(pusha_t*)' + ptr_str(tcb) + '->esp')
   regstate = dict()
   type, regstate['edi'] = eval_expr(cpu, ptr_str(regstate_p) + '->edi')
   type, regstate['esi'] = eval_expr(cpu, ptr_str(regstate_p) + '->esi')
   type, regstate['ebp'] = eval_expr(cpu, ptr_str(regstate_p) + '->ebp')
   type, regstate['esp'] = eval_expr(cpu, ptr_str(regstate_p) + '->original_esp')
   type, regstate['ebx'] = eval_expr(cpu, ptr_str(regstate_p) + '->ebx')
   type, regstate['edx'] = eval_expr(cpu, ptr_str(regstate_p) + '->edx')
   type, regstate['ecx'] = eval_expr(cpu, ptr_str(regstate_p) + '->ecx')
   type, regstate['eax'] = eval_expr(cpu, ptr_str(regstate_p) + '->eax')
   return regstate

# Okay - we can do anything. I'm going to work on something that finds the
#  eip in the iret frame and reports where each runnable thread was 
#  interrupted by the timer. 

##########################

def print_runlist(): 
   runnable = get_runnable()
   print runnable
   
   def print_tail(tcb): 
      type, val = tcb
      if val == 0: 
         print 'ERROR - THIS SHOULD NEVER HAPPEN' 
         return
      if tcb == runnable:
         return 
      
      print regstate_of(tcb)
      print_tail(next_tcb(tcb))
   print_tail(next_tcb(runnable))

def walk_global_list():
   global_pcb = get_global()
   def print_tail(pcb): 
      type, val = pcb
      if val == 0: 
         print 'ERROR - THIS SHOULD NEVER HAPPEN' 
         return
      print pcb
      if pcb == global_pcb:
         return 
      print_tail(next_pcb(pcb))
   
   print_tail(next_pcb(global_pcb))
   
