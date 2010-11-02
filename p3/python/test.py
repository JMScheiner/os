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

def tid_of(tcb):
   type, tid = eval_expr(cpu, ptr_str(tcb) + '->tid')
   return tid

def get_runnable():
   return eval_expr(cpu, 'runnable')

##########################

def print_runlist(): 
   runnable = get_runnable()
   
   def print_tail(tcb): 
      if tcb == runnable:
         return 
      
      print tid_of(tcb)
      print_tail(next_tcb(tcb))
   
   print_tail(next_tcb(runnable))

print_runlist()

