
#ifndef PAGEFAULT_2T3M3QNV

#define PAGEFAULT_2T3M3QNV

void txt_fault(void* addr, int access_mode);
void rodata_fault(void* addr, int access_mode);
void dat_fault(void* addr, int access_mode);
void bss_fault(void* addr, int access_mode);
void stack_fault(void* addr, int access_mode);

void txt_free(region_t* region);
void rodata_free(region_t* region);
void dat_free(region_t* region);
void bss_free(region_t* region);
void stack_free(region_t* region);

#endif /* end of include guard: PAGEFAULT_2T3M3QNV */



