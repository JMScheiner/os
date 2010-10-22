
#include <memman.h>
#include <reg.h>
#include <mm.h>
#include <simics.h>
#include <pagefault.h>
#include <process.h>

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
   void* base, *addr, *arg_addr;
   
   arg_addr = (void*)SYSCALL_ARG(reg);
   
   /* Verify that the arguments lie in valid memory. */
	if(!mm_validate(arg_addr) || !mm_validate(arg_addr + sizeof(void *)))
		RETURN(NEW_PAGES_INVALID_ARGS);
   
   /* Grab the arguments from user space. */
   base = (void*)arg_addr;
   len  = (int)(arg_addr + sizeof(void*));

   if((PAGE_OFFSET(base) != 0) || (len % PAGE_SIZE != 0))
      RETURN(NEW_PAGES_INVALID_ARGS);
   
   /* Check that the pages the user is asking for aren't already already allocated. */
   for(addr = base; addr < base + len; addr += PAGE_SIZE)
      if(mm_validate(addr))
         RETURN(NEW_PAGES_INVALID_ARGS);
   
   /* Check that the pages aren't in the autostack region. */
   if(addr >= (void*)USER_STACK_START) 
      RETURN(NEW_PAGES_INVALID_ARGS);
   
   mm_alloc(get_pcb(), base, len, PTENT_USER | PTENT_RW);
   RETURN(0);
}

void remove_pages_handler(volatile regstate_t reg)
{
	lprintf("Ignoring remove_pages");
	MAGIC_BREAK;
   //TODO
}

