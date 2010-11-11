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
void page_fault_handler(volatile regstate_error_t reg)
{
   int ecode; 
   void* addr;
   pcb_t* pcb;
   region_t* region;
   
   /* The address that causes a page fault resides in cr2.*/
   addr = (void*)get_cr2();
   enable_interrupts();

   pcb = get_pcb();
   ecode = reg.ecode;
   
   /* Our kernel does not page fault. */
   assert(ecode & PF_ECODE_USER);
   assert(!(ecode & PF_ECODE_RESERVED));
   
   void (*handler)(void*, int);

   mutex_lock(&pcb->region_lock);
   for(region = pcb->regions; region; region = region->next)
   {
      if(region->start <= addr && addr < region->end)
      {
			debug_print("page", "fault at %p being handled by region %p with start %p and end %p", addr, region, region->start, region->end);
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

void txt_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Page Fault: Illegal access to .txt region at %p.", addr);
   thread_kill(errbuf);
}

void rodata_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Page Fault: Illegal access to .rodata region at %p.", addr);
   thread_kill(errbuf);
}

void dat_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Page Fault: Illegal access to .data region at %p.", addr);
   thread_kill(errbuf);
}

void bss_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
   sprintf(errbuf, "Page Fault: Illegal access to .bss region at %p.", addr);
   thread_kill(errbuf);
}

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

void stack_fault(void* addr, int ecode)
{
   debug_print("page", "Growing Stack to %p!!!", (void*)PAGE_OF(addr));
	assert(0xb0000000 <= (unsigned int)addr && 
			(unsigned int)addr < 0xc0000000);
   if(mm_alloc(get_pcb(), (void*)PAGE_OF(addr), 
         PAGE_SIZE, PTENT_USER | PTENT_RW) < 0)
   {
      /* Not being able to grow the stack is a fatal problem. */
      thread_kill("Fatal: System ran out of resources on stack allocation");
   }
}

void generic_fault(void* addr, int ecode)
{
   char errbuf[ERRBUF_SIZE];
	assert((unsigned int)addr < 0xb0000000 || 
			(unsigned int)addr >= 0xc0000000);
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



