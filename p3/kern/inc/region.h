/** 
* @file region.h
* @brief The region list is responsible for 
*  deploying page fault handlers, and generally keeping
*  track of how user memory is laid out.
*
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-11-12
*/
#ifndef REGION_M98BMIN2

#define REGION_M98BMIN2

#include <stdlib.h>
#include <stdint.h>
#include <process.h>
#include <kernel_types.h>

int allocate_region( 
   void *start,   
   void *end, 
   int access_level, 
   void (*fault)(void*, int), 
   pcb_t* pcb;
); 

int allocate_stack_region(pcb_t* pcb);
region_t* duplicate_region_list(pcb_t* pcb);
void free_region_list(pcb_t* pcb);
int free_region(pcb_t* pcb, void* start);
boolean_t region_overlaps(pcb_t* pcb, void* start, void* end);

#endif /* end of include guard: REGION_M98BMIN2 */
