
#include <memman.h>
#include <reg.h>
#include <mm.h>
#include <simics.h>

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
   //void* base;
   //int len;
	lprintf("Ignoring new_pages");
	MAGIC_BREAK;
   //TODO
}

void remove_pages_handler(volatile regstate_t reg)
{
	lprintf("Ignoring remove_pages");
	MAGIC_BREAK;
   //TODO
}

