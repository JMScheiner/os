#include <region.h>
#include <kernel_types.h>

#include <mm.h>
#include <simics.h>
#include <malloc.h>
#include <mutex.h>
#include <pagefault.h>
#include <string.h>

int allocate_region( 
   void *start,   
   void *end, 
   int access_level, 
   void (*fault)(void*, int), 
   pcb_t* pcb
) 
{
   /* TODO When does the region list get freed. */
   region_t* region = malloc(sizeof(region_t));
   
   region->fault = fault;
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
   region->start = (void*)USER_STACK_START;
   region->end = (void*)USER_STACK_BASE;

   mutex_lock(&pcb->region_lock);
   region->next = pcb->regions;
   pcb->regions = region;
   mutex_unlock(&pcb->region_lock);
   
	mm_alloc(pcb, (void *)(USER_STACK_BASE - PAGE_SIZE), PAGE_SIZE, PTENT_RW | PTENT_USER);
   return 0;
}

/** 
* @brief Duplicates the region list in pcb and returns a pointer
*  to the copied region list.
*
*  TODO CLEAN ME UP
* 
* @param pcb The process to copy the region list from.
* 
* @return A pointer to the new region list.
*/
region_t* duplicate_region_list(pcb_t* pcb)
{
   region_t *new_iter, *old_iter; 
   region_t *new_head, *old_head; 
   
   mutex_lock(&pcb->region_lock);
   old_head = pcb->regions;
   if(!old_head) 
   {
      mutex_unlock(&pcb->region_lock);
      return NULL;
   }

   new_head = (region_t*)malloc(sizeof(region_t));
   memcpy(new_head, pcb->regions, sizeof(region_t));

   /* The new list trails the old one by one. */
   for(old_iter = old_head, new_iter = new_head->next; 
      old_iter != NULL; 
      old_iter = old_iter->next)
   {
      new_head = new_head->next = (region_t*)malloc(sizeof(region_t));   
      memcpy(new_head, old_head, sizeof(region_t));
   }
   new_iter->next = NULL;
   mutex_unlock(&pcb->region_lock);
   return new_head;
}


