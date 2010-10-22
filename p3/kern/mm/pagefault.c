#include <reg.h>
#include <simics.h>
#include <process.h>
#include <cr.h>
#include <mutex.h>

#define PF_ECODE_NOT_PRESENT 0x1
#define PF_ECODE_WRITE 0x2
#define PF_ECODE_SUPERVISOR 0x4
#define PF_ECODE_RESERVED 0x8

void page_fault_handler(regstate_error_t reg)
{
   int ecode; 
   void* address;
   pcb_t* pcb;
   region_t* region;
   boolean_t region_found;

   pcb = get_pcb();
   ecode = reg.ecode;

   /* The address that causes a page fault resides in cr2.*/
   address = (void*)get_cr2();
   
   assert(!(ecode & PF_ECODE_SUPERVISOR));
   assert(!(ecode & PF_ECODE_RESERVED));
   
   /* Walk the region list, searching for handlers. */
   region_found = FALSE;

   void (*handler)(void*, int);

   mutex_lock(&pcb->region_lock);
   for(region = pcb->regions; region; region = region->next)
   {
      if(region->start < address && address < region->end)
      {
         region_found = TRUE;
         handler = region->handler;
         mutex_unlock(&pcb->region_lock);
         handler(addr, 0);
      }
   }

   if(!region_found)
   {
      mutex_unlock(&pcb->region_lock);
      /* TODO These lprintf's should be turned into printf's when appropriate */
      if(ecode & PF_ECODE_NOT_PRESENT)
         lprintf("Page Fault: Page of %p not present.", addr);
      else if(ecode & PF_ECODE_WRITE)
         lprintf("Page Fault: Illegal write to %p.", addr);
      else assert(0);

      /* TODO The user should be killed!!! */
      MAGIC_BREAK;
   }
}

void txt_fault(void* addr, int access_mode){}

void rodata_fault(void* addr, int access_mode){}

void dat_fault(void* addr, int access_mode){}

void bss_fault(void* addr, int access_mode){}

void stack_fault(void* addr, int access_mode)
{
   /* We should auto allocate the stack region */
   lprintf("Growing Stack!!!");
   mm_alloc(get_pcb(), PAGE_OF(addr), PAGE_SIZE, PTENT_USER | PTENT_RW);
}



