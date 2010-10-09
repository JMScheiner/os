
#include <handlers/handler_wrappers.h>
#include <interrupt_defines.h>
#include <asm.h>
#include <simics.h>
#include <reg.h>

void divide_error_handler(regstate_t reg) 
{
   lprintf(" Made it to the divide by zero handler! " );
   
   lprintf("edi = 0x%x, esi = 0x%x, old_esp= 0x%x, ebp = 0x%x", 
      (int)reg.edi, (int)reg.esi, (int)reg.original_esp, (int)reg.ebp);
   
   lprintf("ebx = 0x%x, edx = 0x%x, ecx = 0x%x, eax = 0x%x", 
      (int)reg.ebx, (int)reg.edx, (int)reg.ecx, (int)reg.eax);
   
   lprintf("eip = 0x%x, cs = 0x%x, eflags = 0x%x", 
      (int)reg.eip, (int)reg.cs, (int)reg.eflags);
   
   lprintf("esp = 0x%x, ss = 0x%x",
      (int)reg.esp, (int)reg.ss);

   lprintf(" ");
   
   MAGIC_BREAK;
	
   outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

void debug_handler(void) {

}

void breakpoint_handler(void) {

}

void overflow_handler(void) {

}

void bound_range_exceeded_handler(void) {

}

void invalid_opcode_handler(void) {

}

void device_not_available_handler(void) {

}

void double_fault_handler(void) {

}

void invalid_tss_handler(void) {

}

void segment_not_present_handler(void) {

}

void stack_segment_fault_handler(void) {

}

void general_protection_handler(void) {

}

void page_fault_handler(void) {

}

void alignment_check_handler(void) {

}

void machine_check_handler(void) {

}

