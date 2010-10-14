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

   void (*fault)(struct REGION* region, void* addr, int access_mode);
   void (*free)(struct REGION* region);
   struct REGION* next;
};

int allocate_region( 
   void *start,   
   void *end, 
   int access_level, 
   void (*fault)(region_t*, void*, int), 
   void (*free)(region_t*),
   pcb_t* pcb;
); 

void* init_bss(char* bss, size_t size);
void* init_data(char* data, size_t size);
void* init_text(char* text, size_t size);
void* init_stack(char* stack, size_t size);

#endif /* end of include guard: REGION_M98BMIN2 */
