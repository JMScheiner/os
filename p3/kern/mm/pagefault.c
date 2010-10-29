#include <reg.h>
#include <process.h>
#include <cr.h>
#include <mutex.h>
#include <mm.h>
#include <simics.h>
#include <debug.h>

#define PF_ECODE_NOT_PRESENT 0x1
#define PF_ECODE_WRITE 0x2
#define PF_ECODE_USER 0x4
#define PF_ECODE_RESERVED 0x8

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
         handler(addr, 0);
      }
   }

   if(!region_found)
   {
      mutex_unlock(&pcb->region_lock);
      /* TODO These lprintf's should be turned into printf's when appropriate */
      if(ecode & PF_ECODE_NOT_PRESENT)
         debug_print("page", "Page Fault: Page of %p not present.", addr);
      else if(ecode & PF_ECODE_WRITE)
         debug_print("page", "Page Fault: Illegal write to %p.", addr);
      else assert(0);

      /* TODO The user should be killed!!! */
      debug_break("page");
   }
}

void txt_fault(void* addr, int access_mode){}

void rodata_fault(void* addr, int access_mode){}

void dat_fault(void* addr, int access_mode){}

void bss_fault(void* addr, int access_mode){}

void stack_fault(void* addr, int access_mode)
{
   /* We should auto allocate the stack region */
   debug_print("page", "Growing Stack to %p!!!", (void*)PAGE_OF(addr));
   mm_alloc(get_pcb(), (void*)PAGE_OF(addr), PAGE_SIZE, PTENT_USER | PTENT_RW);
}



