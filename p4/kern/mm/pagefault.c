/** 
* @file pagefault.c
* @brief Page fault handlers + ZFOD. 
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-11-12
*/
#include <reg.h>
#include <process.h>
#include <cr.h>
#include <mutex.h>
#include <mm.h>
#include <simics.h>
#include <debug.h>
#include <lifecycle.h>
#include <stdio.h> /* sprintf */
#include <ecodes.h>
#include <asm.h>
#include <ureg.h>
#include <swexn.h>
#include <idt.h>

#define PF_ECODE_NOT_PRESENT 0x1
#define PF_ECODE_WRITE 0x2
#define PF_ECODE_USER 0x4
#define PF_ECODE_RESERVED 0x8
#define ERRBUF_SIZE 0x100

void generic_fault(void* addr, int ecode);

/** 
* @brief Page fault handler. 
*
*  Determines what region of user memory caused the fault, 
*   and dispatches the appropriate handler. 
*  
*  On entry, interrupts are disabled, so %cr2 doesn't change
*     (as a result of another page fault.)
* 
* @param reg The register state on entry to the handler.
*/
void page_fault_handler(ureg_t* reg)
{
   int ecode; 
   void* addr;
   pcb_t* pcb;
   region_t* region;
   
   /* The address that causes a page fault resides in cr2.*/
   addr = (void*)reg->cr2;
   ecode = reg->error_code;

   pcb = get_pcb();
   
   /* Our kernel does not page fault. */
   assert(ecode & PF_ECODE_USER);
   assert(!(ecode & PF_ECODE_RESERVED));
   
   swexn_try_invoke_handler(reg);
   
   void (*handler)(void*, int);

   mutex_lock(&pcb->region_lock);
   for(region = pcb->regions; region; region = region->next)
   {
      if(region->start <= addr && addr < region->end)
      {
         debug_print("page",
            "fault at %p being handled by region %p with start %p and end %p",
            addr, region, region->start, region->end);
         handler = region->fault;
         mutex_unlock(&pcb->region_lock);
         handler(addr, ecode);
         return;
      }
   }
   mutex_unlock(&pcb->region_lock);

   debug_print("page", "fault at %p being handled by generic fault", addr);
   generic_fault(addr, ecode);
}

/** 
* @brief The fault handler invoked by a page fault on the .txt region. 
* 
* @param addr The address that caused the fault. 
* @param ecode The error code of the fault. 
*/
void txt_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   debug_print("page", ".txt fault at %p!!!", addr);
   sprintf(errbuf, "Page Fault: Illegal access to .txt region at %p.", addr);
   thread_kill(errbuf);
}

/** 
* @brief The fault handler invoked by a page fault on the .rodat region. 
* 
* @param addr The address that caused the fault. 
* @param ecode The error code of the fault. 
*/
void rodata_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   debug_print("page", ".rodat fault at %p!!!", addr);
   sprintf(errbuf, 
      "Page Fault: Illegal access to .rodata region at %p.", addr);
   thread_kill(errbuf);
}

/** 
* @brief The fault handler invoked by a page fault on the .dat region. 
* 
* @param addr The address that caused the fault. 
* @param ecode The error code of the fault. 
*/
void dat_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   debug_print("page", ".dat fault at %p!!!", addr);
   sprintf(errbuf, "Page Fault: Illegal access to .data region at %p.", addr);
   thread_kill(errbuf);
}

/** 
* @brief The fault handler invoked by a page fault on the .bss region. 
* 
* @param addr The address that caused the fault. 
* @param ecode The error code of the fault. 
*/
void bss_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   debug_print("page", "bss fault at %p!!!", addr);
   
   if(ecode & PF_ECODE_WRITE)
   {
      debug_print("page", "Framing ZFOD page!", addr);
      mm_frame_zfod_page(addr);
      return;
   }

   sprintf(errbuf, "Page Fault: Illegal access to .bss region at %p.", addr);
   thread_kill(errbuf);
}

/** 
* @brief The fault handler invoked by a new_pages'd region. 
* 
* @param addr The address that caused the fault. 
* @param ecode The error code of the fault. 
*/
void user_fault(void* addr, int ecode)
{
   /* In our implementation this shouldn't happen. 
    *
    *  It is the fault handler for memory that has been new_pages'd. 
    *   Since new_pages'd regions don't overlap with existing regions, 
    *   and are allocated user r/w, they do not fault. 
    *    (unless things are going wrong.)
    **/
   assert(0);
}

/** 
* @brief The fault handler invoked by a page fault in the stack region. 
*
*  Since stack frames are generally user R/W - this is generally impossible
*   under our currently non-fancy framework.
*
* @param addr The address that caused the fault. 
* @param ecode The error code of the fault. 
*/
void stack_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   debug_print("page", "stack fault at %p!!!", addr);
   sprintf(errbuf, "Page Fault: Illegal access to stack region at %p.", addr);
   thread_kill(errbuf);
}

/** 
* @brief A generic fault handler for addresses that don't lie in
*  a named region. 
* 
* @param addr The address that caused the fault. 
* @param ecode The error code of the fault. 
*/
void generic_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   debug_print("page", "Generic fault at %p!!!", addr);
   
   if(!(ecode & PF_ECODE_NOT_PRESENT))
   {
      sprintf(errbuf, "Page Fault: %p not present in memory.", addr);
      thread_kill(errbuf);
   } 
   else if(ecode & PF_ECODE_WRITE)
   {
      sprintf(errbuf, "Page Fault: Illegal write to %p.", addr);
      thread_kill(errbuf);
   }
   else
   {
      sprintf(errbuf, "Page Fault: Illegal read from %p.", addr);
      thread_kill(errbuf);
   }
}



