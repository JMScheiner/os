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

#endif /* end of include guard: REGION_M98BMIN2 */
