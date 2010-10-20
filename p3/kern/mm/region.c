
#include <region.h>
#include <malloc.h>
#include <mm.h>
#include <process.h>
#include <simics.h>

int allocate_region( 
   void *start,   
   void *end, 
   int access_level, 
   void (*fault)(region_t*, void*, int), 
   void (*free)(region_t*),
   pcb_t* pcb
) 
{
   
   /* TODO When does the region list get freed. */
   region_t* region = malloc(sizeof(region_t));
   
   region->fault = fault;
   region->free = free;
   region->start = start;
   region->end = end;

   /* TODO Race condition here: */
   region->next = pcb->regions;
   pcb->regions = region;
   
   lprintf("Allocating %p, access %x", start, access_level);
	mm_alloc(pcb, (void *)start, end - start, access_level);
	return 0;
}




