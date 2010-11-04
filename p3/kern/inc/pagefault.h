
#ifndef PAGEFAULT_2T3M3QNV

#define PAGEFAULT_2T3M3QNV

void txt_fault(void* addr, int access_mode);
void rodata_fault(void* addr, int access_mode);
void dat_fault(void* addr, int access_mode);
void bss_fault(void* addr, int access_mode);
void stack_fault(void* addr, int access_mode);
void user_fault(void* addr, int access_mode);


#endif /* end of include guard: PAGEFAULT_2T3M3QNV */



