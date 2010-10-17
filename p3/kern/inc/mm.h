

#ifndef MM_1PZ6H5QE

#define MM_1PZ6H5QE

#include <stdlib.h>
#include <page.h>
#include <types.h>
#include <process.h>

#define PTENT_PRESENT      0x1
#define PTENT_RO           0x0
#define PTENT_RW           0x2
#define PTENT_SUPERVISOR   0x0
#define PTENT_USER         0x4
#define PTENT_COW          0x200
#define PTENT_ZFOD         0x400

#define PAGE_OF(addr) (((int)addr) & (~(PAGE_SIZE - 1)))
#define PAGE_MASK (PAGE_SIZE - 1)
#define PAGE_OFFSET(addr) (((int)addr) & PAGE_MASK)

/** @brief Evaluate to true iff addr1 and addr2 are on the same page. 
 *
 * @param addr1 The first address
 * @param addr2 The second address
 *
 * @return True iff addr1 and addr2 are on the same page.
 */
#define SAME_PAGE(addr1, addr2) \
	(PAGE_OF(addr1) == PAGE_OF(addr2))

int mm_init(void); 

void* mm_new_directory(void);
void* mm_new_table(void);
void mm_alloc(pcb_t* pcb, void* addr, size_t len, unsigned int flags);
void mm_free_pages(pcb_t* pcb, void* addr, size_t n);
void* mm_new_kernel_pages(size_t n);
void mm_duplicate_address_space(void* dir);

int mm_getflags(void* addr);
boolean_t mm_validate(void* addr);

#endif /* end of include guard: MM_1PZ6H5QE */

