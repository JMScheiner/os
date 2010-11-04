#include <reg.h>
#include <process.h>
#include <cr.h>
#include <mutex.h>
#include <mm.h>
#include <simics.h>
#include <debug.h>
#include <lifecycle.h>
#include <stdio.h> /* sprintf */

#define PF_ECODE_NOT_PRESENT 0x1
#define PF_ECODE_WRITE 0x2
#define PF_ECODE_USER 0x4
#define PF_ECODE_RESERVED 0x8
#define ERRBUF_SIZE 0x100

void generic_fault(int ecode, void* addr);

void page_fault_handler(volatile regstate_error_t reg)
{
   int ecode; 
   void* addr;
   pcb_t* pcb;
   region_t* region;
   boolean_t region_found;

   pcb = get_pcb();
   ecode = reg.ecode;

   /* The address that causes a page fault resides in cr2.*/
   addr = (void*)get_cr2();
   
   assert(ecode & PF_ECODE_USER);
   assert(!(ecode & PF_ECODE_RESERVED));
   
   /* Walk the region list, searching for handlers. */
   region_found = FALSE;

   void (*handler)(void*, int);

   mutex_lock(&pcb->region_lock);
   for(region = pcb->regions; region; region = region->next)
   {
      if(region->start < addr && addr < region->end)
      {
         region_found = TRUE;
         handler = region->fault;
         mutex_unlock(&pcb->region_lock);
         handler(addr, ecode);
      }
   }

   if(!region_found)
   {
      mutex_unlock(&pcb->region_lock);
      generic_fault(ecode, addr);
   }
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

void stack_fault(void* addr, int ecode)
{
   debug_print("page", "Growing Stack to %p!!!", (void*)PAGE_OF(addr));
   mm_alloc(get_pcb(), (void*)PAGE_OF(addr), PAGE_SIZE, PTENT_USER | PTENT_RW);
}

void generic_fault(int ecode, void* addr)
{
   char errbuf[ERRBUF_SIZE];
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



