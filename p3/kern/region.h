#ifndef REGION_M98BMIN2

#define REGION_M98BMIN2

void* init_bss(char* bss, size_t size);
void* init_data(char* data, size_t size);
void* init_text(char* text, size_t size);
void* init_stack(char* stack, size_t size);

#endif /* end of include guard: REGION_M98BMIN2 */
