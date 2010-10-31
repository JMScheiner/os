
#include <memman.h>
#include <reg.h>
#include <mm.h>
#include <simics.h>
#include <pagefault.h>
#include <process.h>
#include <mutex.h>
#include <vstring.h>

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
   char* base, *addr, *arg_addr;
   
   arg_addr = (void*)SYSCALL_ARG(reg);
   if(v_memcpy((char*)&base, arg_addr, sizeof(char*)) < sizeof(char*))
      RETURN(NEW_PAGES_INVALID_ARGS);
   
   if(v_memcpy((char*)&len, arg_addr + sizeof(char*), sizeof(int)) < sizeof(int))
      RETURN(NEW_PAGES_INVALID_ARGS);

   if((PAGE_OFFSET(base) != 0) || (len % PAGE_SIZE != 0))
      RETURN(NEW_PAGES_INVALID_ARGS);
   
   /* Check that the pages the user is asking for aren't already already allocated. */
   for(addr = base; addr < base + len; addr += PAGE_SIZE)
      if(mm_validate_read(base, PAGE_SIZE))
         RETURN(NEW_PAGES_INVALID_ARGS);
   
   /* Check that the pages aren't in the autostack region. 
    *  Or in reserved kernel virtual memory. */
   if((addr > (char*)USER_STACK_START && addr < (char*)USER_STACK_BASE) || 
      (base > (char*)USER_STACK_START && base < (char*)USER_STACK_BASE) ||
      (addr > (char*)USER_MEM_END))
      RETURN(NEW_PAGES_INVALID_ARGS);
   
   mm_alloc(get_pcb(), base, len, PTENT_USER | PTENT_RW);
   RETURN(0);
}

void remove_pages_handler(volatile regstate_t reg)
{
   mutex_lock(&_remove_pages_lock);
	lprintf("Ignoring remove_pages");
	MAGIC_BREAK;
   mutex_unlock(&_remove_pages_lock);
}

