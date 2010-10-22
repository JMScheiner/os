
#include <process.h>
#include <malloc.h>
#include <region.h>
#include <mm.h>
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

   mutex_lock(&pcb->region_lock);
   region->next = pcb->regions;
   pcb->regions = region;
   mutex_unlock(&pcb->region_lock);
   
	mm_alloc(pcb, (void *)start, end - start, access_level);
	return 0;
}

int allocate_stack_region(pcb_t* pcb)
{
   region_t* region = malloc(sizeof(region_t));
   
   region->fault = stack_fault;
   region->free = stack_free;
   region->start = USER_STACK_START;
   region->end = USER_STACK_BASE;

   mutex_lock(&pcb->region_lock);
   region->next = pcb->regions;
   pcb->regions = region;
   mutex_unlock(&pcb->region_lock);
   
	mm_alloc(pcb, (void *)(USER_STACK_BASE - PAGE_SIZE), PAGE_SIZE, PTENT_RW | PTENT_USER);
   return 0;
}




