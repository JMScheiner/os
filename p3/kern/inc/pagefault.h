
#ifndef PAGEFAULT_2T3M3QNV

#define PAGEFAULT_2T3M3QNV

void txt_fault(void* addr, int access_mode);
void rodata_fault(void* addr, int access_mode);
void dat_fault(void* addr, int access_mode);
void bss_fault(void* addr, int access_mode);
void stack_fault(void* addr, int access_mode);

void txt_free(void* addr);
void rodata_free(void* addr);
void dat_free(void* addr);
void bss_free(void* addr);
void stack_free(void* addr);

#endif /* end of include guard: PAGEFAULT_2T3M3QNV */



