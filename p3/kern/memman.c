
#include <memman.h>
#include <reg.h>
#include <mm.h>
#include <simics.h>
#include <pagefault.h>
#include <process.h>
#include <mutex.h>
#include <vstring.h>
#include <region.h>
#include <common_kern.h>
#include <syscall_codes.h>
#include <debug.h>

/* @brief Protects us from the user removing pages while we try 
 *  to execute a validated copy from user space. */
mutex_t _remove_pages_lock;
inline mutex_t* remove_pages_lock(){ return &_remove_pages_lock; }

void memman_init()
{
   mutex_init(&_remove_pages_lock);
}

/** 
* @brief Allocates new memory to the invoking task, starting at base 
*  and extending for len bytes.
*
* new_pages() will fail, returning a negative integer error code, if 
*  base is not page-aligned, if len is not a positive integral multiple 
*  of the system page size, if any portion of the region already represents 
*  memory in the tasks address space, if the new memory region would be too 
*  close to the bottom of the automatic stack region, or if the operating 
*  system has insufficient resources to satisfy the request.
*
* @param reg The register state on entry to the handler.
*/
void new_pages_handler(volatile regstate_t reg)
{
   int len;
   char* start, *addr, *arg_addr, *end;
   
   arg_addr = (void*)SYSCALL_ARG(reg);
   if(v_memcpy((char*)&start, arg_addr, sizeof(char*)) < sizeof(char*))
      RETURN(NEW_PAGES_INVALID_ARGS);
   
   if(v_memcpy((char*)&len, arg_addr + sizeof(char*), sizeof(int)) < sizeof(int))
      RETURN(NEW_PAGES_INVALID_ARGS);

   if((PAGE_OFFSET(start) != 0) || (len % PAGE_SIZE != 0))
      RETURN(NEW_PAGES_INVALID_ARGS);
   
   /* Check that the pages the user is asking for aren't already 
    * already allocated. */
   for(addr = start; addr < start + len; addr += PAGE_SIZE)
      if(mm_validate_read(start, PAGE_SIZE))
         RETURN(NEW_PAGES_INVALID_ARGS);
   
   end = addr;
   
   /* Check that the pages aren't in the autostack region. 
    *  Or in reserved kernel virtual memory. */
   if((end > (char*)USER_STACK_START && end < (char*)USER_STACK_BASE) || 
      (start > (char*)USER_STACK_START && start < (char*)USER_STACK_BASE) ||
      (start < (char*)USER_MEM_START || end > (char*)USER_MEM_END))
      RETURN(NEW_PAGES_INVALID_ARGS);
   
   debug_print("memman", " Allocating new region [%p, %p] for new_pages", start, end);
   allocate_region(start, end, PTENT_USER | PTENT_RW, user_fault, get_pcb());
   RETURN(0);
}

/* @brief Deallocates the specified memory region, which must presently be 
 *    allocated as the result of a previous call to new pages() which 
 *    specified the same value of base. Returns zero if successful or 
 *    returns a negative integer failure code.
* 
* @param reg The register state on entry to new_pages. 
*/
void remove_pages_handler(volatile regstate_t reg)
{
   region_t* region, *last_region = NULL;
   pcb_t* pcb;
   void* start, *end;
   
   start = (char*)SYSCALL_ARG(reg);
   pcb = get_pcb();

   mutex_lock(&_remove_pages_lock);
   mutex_lock(&pcb->region_lock);
   for(region = pcb->regions; region; region = region->next)
   {
      if(region->start == start && (region->fault == user_fault))
      {
         /* Remove the region from the region list. */
         if(last_region == NULL)
            pcb->regions = region->next;
         else
            last_region->next = region->next;
         
         end = region->end;
         sfree(region, sizeof(region_t));
         mutex_unlock(&pcb->region_lock);
         mutex_unlock(&_remove_pages_lock);

         /* Free the memory associated with the region. */
         debug_print("memman", " Removing region [%p, %p] for remove_pages", start, end);
         mm_remove_pages(pcb, start, end);
         RETURN(0);
      }
      else last_region = region;
   }
   
   mutex_unlock(&pcb->region_lock);
   mutex_unlock(&_remove_pages_lock);
   RETURN(REMOVE_PAGES_REGION_NOT_FOUND);

}



