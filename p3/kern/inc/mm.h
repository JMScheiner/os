

#ifndef MM_1PZ6H5QE

#define MM_1PZ6H5QE

#include <stdlib.h>

int mm_init(void); 

void* mm_new_directory(void);
void* mm_new_table(void);
void* mm_new_pages(void* addr, size_t n);
void* mm_new_kernel_pages(size_t n);

#endif /* end of include guard: MM_1PZ6H5QE */

