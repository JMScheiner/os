/** 
* @file region.c
* @brief A simple region list manager. 
* 
*  The main utility of keeping a region list for every process is to 
*   be able to dispatch the correct page fault handler for a given 
*   region.  While our page fault handler is relatively simple, 
*   a region list will make it easier to implement the autostack, 
*   ZFOD, and COW.
*
* @author Justin Scheiner
* @author Tim Wilson 
* @date 2010-11-03
*/
#include <region.h>
#include <kernel_types.h>

#include <mm.h>
#include <simics.h>
#include <malloc.h>
#include <mutex.h>
#include <pagefault.h>
#include <string.h>
#include <debug.h>

/** 
* @brief Allocates a new region in the address space in PCB.
* 
* @param start The starting address of the region. 
* @param end The ending address of the region.
* @param access_level The (flags) to give the region. (e.g. PTENT_RW)
* @param void(*fault)(void*, int), The page fault handler for this region.
* 
* @return 0 on success. 
*/
int allocate_region( 
   void *start,   
   void *end, 
   int access_level, 
   void (*fault)(void*, int), 
   pcb_t* pcb
) 
{
   region_t* region = smalloc(sizeof(region_t));
   memset(region, 0, sizeof(region_t));
   
   debug_print("region", "Allocated new region [%p, %p] at %p", 
      start, end, region);
   
   region->fault = fault;
   region->start = start;
   region->end = end;

   mutex_lock(&pcb->region_lock);
   if(pcb->regions == region)
      MAGIC_BREAK;
   region->next = pcb->regions;
   pcb->regions = region;
   mutex_unlock(&pcb->region_lock);
   
	mm_alloc(pcb, (void *)start, end - start, access_level);
	return 0;
}

/** 
* @brief Declare the stack region to be a large region below the stack base, 
*  but only allocate one frame for the new process. 
* 
* @param pcb The process / address space to allocate the stack region for. 
* 
* @return 0 on success. 
*/
int allocate_stack_region(pcb_t* pcb)
{
   region_t* region = (region_t*)smalloc(sizeof(region_t));
   
   region->fault = stack_fault;
   region->start = (void*)USER_STACK_START;
   region->end = (void*)USER_STACK_BASE;

   mutex_lock(&pcb->region_lock);
   region->next = pcb->regions;
   pcb->regions = region;
   mutex_unlock(&pcb->region_lock);
   
   debug_print("region", "Allocated stack region [%p, %p] at %p", 
      (void*)USER_STACK_START, (void*)USER_STACK_BASE), region;
	
   mm_alloc(pcb, (void *)(USER_STACK_BASE - PAGE_SIZE), 
      PAGE_SIZE, PTENT_RW | PTENT_USER);
   
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
   region_t *iter0, *iter1;
   region_t *head0, *head1;

   assert(pcb->regions);
   
   mutex_lock(&pcb->region_lock);
   
   head0 = pcb->regions;
   head1 = smalloc(sizeof(region_t));
   
   iter0 = head0; iter1 = head1;
   for(;;)
   {
      memcpy(iter1, iter0, sizeof(region_t));
      debug_print("region", "Duplicated region [%p, %p] at %p", 
         iter0->start, iter0->end, iter1);
      
      if(iter0->next) 
      {
         iter1->next = smalloc(sizeof(region_t));
         iter0 = iter0->next; 
         iter1 = iter1->next;
      }
      else
      {
         iter1->next = NULL;
         break;
      }
   }

   mutex_unlock(&pcb->region_lock);
   return head1;
}

/** 
* @brief Frees the region list in the given PCB. 
* 
* @param pcb The pcb to free the region list for. 
*/
void free_region_list(pcb_t* pcb)
{
   region_t *iter, *next; 
   assert(pcb->regions);

   mutex_lock(&pcb->region_lock);
   for(iter = pcb->regions; iter != NULL; iter = next)
   {
      debug_print("region", "Freeing region [%p, %p]", iter->start, iter->end);
      next = iter->next;
      sfree(iter, sizeof(region_t));
   }
   pcb->regions = NULL;
   mutex_unlock(&pcb->region_lock);
}

