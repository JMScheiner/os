

#ifndef MM_1PZ6H5QE

#define MM_1PZ6H5QE

#include <stdlib.h>

#define PTENT_RO           0x0
#define PTENT_RW           0x2
#define PTENT_SUPERVISOR   0x0
#define PTENT_USER         0x4

int mm_init(void); 

void* mm_new_directory(void);
void* mm_new_table(void);
int mm_new_pages(void* addr, size_t n, unsigned int flags);
int mm_alloc(void* addr, size_t len, unsigned int flags);
void* mm_new_kernel_pages(size_t n);

#endif /* end of include guard: MM_1PZ6H5QE */

