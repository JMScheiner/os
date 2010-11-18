/** 
* @file memman.c
* @brief Implements new_pages and remove_pages
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-11-12
*/

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
#include <ecodes.h>
#include <debug.h>
#include <eflags.h>

/* @brief Protects us from the user removing pages while we try 
 *  to execute a validated copy from user space. */
mutex_t _new_pages_lock;


/** 
* @brief Return the new_pages lock. 
* 
* @return The new_pages lock. 
*/
inline mutex_t* new_pages_lock(){ return &_new_pages_lock; }

void memman_init()
{
   mutex_init(&_new_pages_lock);
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
   int len, ret;
   char* start, *arg_addr, *end;
   
   arg_addr = (void*)SYSCALL_ARG(reg);

   if(v_copy_in_ptr(&start, arg_addr) < 0)
      RETURN(EARGS);
   
   if(v_copy_in_int(&len, arg_addr + sizeof(char*)) < 0)
      RETURN(EARGS);
   
   end = start + len;
   
   /* Check that the requested memory is in user space. */
   if((start < (char*)USER_MEM_START) || (end > (char*)USER_MEM_END))
      RETURN(EARGS);
   
   /* Check that the requested memory is page aligned. */
   if((PAGE_OFFSET(start) != 0) || (len % PAGE_SIZE != 0))
      RETURN(EARGS);
   
   pcb_t* pcb = get_pcb();
   
   /* Check that the pages the user is asking for aren't already 
    * already allocated. */
   
   assert((get_eflags() & EFL_IF) != 0);
   mutex_lock(&_new_pages_lock);
   if(region_overlaps(pcb, start, end))
   {
      mutex_unlock(&_new_pages_lock);
      RETURN(ESTATE);
   }
   
   debug_print("memman", " Allocating new region [%p, %p] for new_pages", start, end);
   
   if((ret = allocate_region(start, 
      end, PTENT_USER | PTENT_RW, user_fault, get_pcb())) < 0)
   {
      debug_print("memman", "new_pages failure");
      mutex_unlock(&_new_pages_lock);
      RETURN(ret);
   }
   
   mutex_unlock(&_new_pages_lock);
   RETURN(ESUCCESS);
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
   pcb_t* pcb;
   void* start;
   
   start = (char*)SYSCALL_ARG(reg);
   if(start < (void*)USER_MEM_START || start > (void*)USER_MEM_END)
      RETURN(EARGS);
   
   pcb = get_pcb();
   
   int ret;
   mutex_lock(&_new_pages_lock);
   ret = free_region(pcb, start);
   mutex_unlock(&_new_pages_lock);

   RETURN(ret);
}



