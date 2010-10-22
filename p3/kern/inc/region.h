#ifndef REGION_M98BMIN2

#define REGION_M98BMIN2

#include <stdlib.h>
#include <stdint.h>

typedef struct REGION region_t;

#include <process.h>

struct REGION
{
   void* start;
   void* end;

   void (*fault)(void* addr, int access_mode);
   struct REGION* next;
};

int allocate_region( 
   void *start,   
   void *end, 
   int access_level, 
   void (*fault)(void*, int), 
   pcb_t* pcb;
); 

int allocate_stack_region(pcb_t* pcb)

#endif /* end of include guard: REGION_M98BMIN2 */
