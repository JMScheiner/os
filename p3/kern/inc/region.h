#ifndef REGION_M98BMIN2

#define REGION_M98BMIN2

#include <stdlib.h>
#include <stdint.h>

typedef struct REGION
{
   void (*fault)(void* addr, uint8_t access_mode);
   void (*init)(void* addr, uint8_t access_mode);
   void (*free)(void* addr, uint8_t access_mode);
} region_t;

void* init_bss(char* bss, size_t size);
void* init_data(char* data, size_t size);
void* init_text(char* text, size_t size);
void* init_stack(char* stack, size_t size);

#endif /* end of include guard: REGION_M98BMIN2 */
