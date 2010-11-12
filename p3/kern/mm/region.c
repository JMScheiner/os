/** 
* @file region.c
* @brief A simple region list manager. 
*
* The region list helps with management of user memory. 
*
* This region list and this file are responsible for 
*  - allocating regions (which may overlap)
*  - dispatching the correct page fault handler
*  - checking for allocated memory in new_pages
*
* @author Justin Scheiner
* @author Tim Wilson 
* @date 2010-11-03
*/
#include <region.h>
#include <kernel_types.h>
#include <common_kern.h>

#include <mm.h>
#include <simics.h>
#include <malloc.h>
#include <mutex.h>
#include <pagefault.h>
#include <string.h>
#include <debug.h>
#include <ecodes.h>
#include <malloc_wrappers.h>
#include <thread.h>

/** 
* @brief Allocates a new region in the address space in PCB.
* 
* @param start The starting address of the region. 
* @param end The ending address of the region.
* @param access_level The (flags) to give the region. (e.g. PTENT_RW)
* @param void(*fault)(void*, int), The page fault handler for this region.
* 
* @return 0 on success. ENOVM or ENOMEM on failure. 
*/
int allocate_region( 
   void *start,   
   void *end, 
   int access_level, 
   void (*fault)(void*, int), 
   pcb_t* pcb
) 
{
   region_t* region;
   if((region = (region_t*)scalloc(1, sizeof(region_t))) == NULL)
      return ENOMEM;
   
   region->fault = fault;
   region->start = start;
   region->end = end;
   
   int ret;
   if((ret = mm_alloc(pcb, (void*)start, end - start, access_level)) < 0)
   {
      debug_print("region", "Failed to allocate region [%p, %p]", start, end); 
      sfree(region, sizeof(region_t));
      return ret;
   }
   debug_print("region", "Allocated new region [%p, %p] at %p", start, end, region);

   /* Insert the region into the list. */
   mutex_lock(&pcb->region_lock);
   assert(pcb->regions != region);
   region->next = pcb->regions;
   pcb->regions = region;
   mutex_unlock(&pcb->region_lock);
   
	return ESUCCESS;
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
   region_t* region;
   if((region = (region_t*)scalloc(1, sizeof(region_t))) == NULL)
   {
      return ENOMEM;
   }
   
   region->fault = stack_fault;
   region->start = (void*)USER_STACK_START;
   region->end = (void*)USER_STACK_BASE;
   
   int ret;
   if((ret = mm_alloc(pcb, (void *)(USER_STACK_BASE - PAGE_SIZE), 
      PAGE_SIZE, PTENT_RW | PTENT_USER)) < 0)
   {
      sfree(region, sizeof(region_t));
      return ret;
   }

   mutex_lock(&pcb->region_lock);
   region->next = pcb->regions;
   pcb->regions = region;
   mutex_unlock(&pcb->region_lock);
   
   debug_print("region", "Allocated stack region [%p, %p] at %p", 
      (void*)USER_STACK_START, (void*)USER_STACK_BASE), region;
   
   return ESUCCESS;
}

void free_region_list_helper(region_t* regions)
{
   region_t *iter, *next; 
   for(iter = regions; iter != NULL; iter = next)
   {
      debug_print("region", "Freeing region [%p, %p]", iter->start, iter->end);
      next = iter->next;
      sfree(iter, sizeof(region_t));
   }
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
   head1 = scalloc(1, sizeof(region_t));
   if(head1 == NULL)
   {
      mutex_unlock(&pcb->region_lock);
      return NULL;
   }
   iter0 = head0; 
   iter1 = head1;

   for(;;)
   {
      memcpy(iter1, iter0, sizeof(region_t));
      debug_print("region", "Duplicated region [%p, %p] at %p", 
         iter0->start, iter0->end, iter1);
      
      if(iter0->next) 
      {
         iter1->next = scalloc(1, sizeof(region_t));
         if(iter1->next == NULL)
         {
            mutex_unlock(&pcb->region_lock);
            free_region_list_helper(head1);
            return NULL;
         }
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
   assert(pcb->regions);
   mutex_lock(&pcb->region_lock);
   free_region_list_helper(pcb->regions);
   pcb->regions = NULL;
   mutex_unlock(&pcb->region_lock);
}

/** 
* @brief A utility function for region_overlaps.
*/
inline boolean_t region_overlaps_helper(void* start0, void* end0, void* start1, void* end1)
{
   return ((start0 < start1 && start1 < end0) ||
           (start1 < start0 && start0 < end1));
}

/** 
* @brief Returns true if the address range specifies overlaps 
*  with an existing region. 
*   
*   A utility function for new_pages. 
* 
* @param pcb The pcb containing the region list. 
* @param start The starting address of the "proposed" region. 
* @param end The ending address of the "proposed" region. 
* 
* @return True if the region overlaps with an existing region.
*/
boolean_t region_overlaps(pcb_t* pcb, void* start, void* end)
{
   region_t *iter;

   mutex_lock(&pcb->region_lock);
   for(iter = pcb->regions; iter != NULL; iter = iter->next)
   {
      assert((void*)iter < (void*)USER_MEM_START);
      if(region_overlaps_helper(iter->start, iter->end, start, end))
      {
         mutex_unlock(&pcb->region_lock);
         return TRUE;
      }
   }
   mutex_unlock(&pcb->region_lock);
   return FALSE;
}

int free_region(pcb_t* pcb, void* start)
{
   region_t *region, *last_region = NULL;
   void* end;

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

         /* Free the memory associated with the region. 
          *  (Note we are the only thread that knows about it) */
         debug_print("region", " Removing region [%p, %p]", start, end);
         mm_remove_pages(pcb, start, end);
         return 0;
      }
      else last_region = region;
   }
   
   /* The region wasn't found. */
   mutex_unlock(&pcb->region_lock);
   return -1;
}


