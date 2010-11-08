/** 
* @file mm.h
* @brief Defines for users of the memory management library.
* @author Justin Scheiner
* @author Tim Wilson
*/


#ifndef MM_1PZ6H5QE

#define MM_1PZ6H5QE

#include <stdlib.h>
#include <page.h>
#include <types.h>
#include <process.h>

/* The top n MB of addressable space will be used exclusively for kvm */
#define USER_MEM_END 0xF0000000

#define PTENT_PRESENT      0x1
#define PTENT_RO           0x0
#define PTENT_RW           0x2
#define PTENT_SUPERVISOR   0x0
#define PTENT_USER         0x4
#define PTENT_COW          0x200
#define PTENT_ZFOD         0x400

#define PAGE_MASK (PAGE_SIZE - 1)
#define PAGE_OF(addr) (((int)(addr)) & (~PAGE_MASK))
#define FLAGS_OF(addr) (((int)(addr)) & (PAGE_MASK))
#define PAGE_OFFSET(addr) (((int)(addr)) & PAGE_MASK)

/** @brief Evaluate to true iff addr1 and addr2 are on the same page. 
 *
 * @param addr1 The first address
 * @param addr2 The second address
 *
 * @return True iff addr1 and addr2 are on the same page.
 */
#define SAME_PAGE(addr1, addr2) \
	(PAGE_OF(addr1) == PAGE_OF(addr2))

/** @brief Test whether a mask is completely set in the page given 
 * by addr. */
#define TEST_SET(addr, mask) \
	(((addr) & (mask)) == (mask))

/** @brief Test whether a mask is completely unset in the page given 
 * by addr. */
#define TEST_UNSET(addr, mask) \
	(((addr) & (mask)) == 0)

/**
 * @brief Count the number of pages spanned by the range [addr, addr+len]
 *
 * @param addr The base address
 * @param len The number of bytes in the range
 *
 * @return The number of pages in the range
 */
#define NUM_PAGES(addr, len) \
	((PAGE_OF((unsigned long)(addr) + (len) - 1) \
		- PAGE_OF(addr)) / PAGE_SIZE + 1)

/** Initialize **/
int mm_init(void); 

/** Request / receive resources */
void *mm_new_kp_page(void);
int mm_alloc(pcb_t* pcb, void* addr, size_t len, unsigned int flags);
int mm_duplicate_address_space(pcb_t* pcb);
int mm_request_frames(int n);

/** Release resources **/
void mm_remove_pages(pcb_t* pcb, void* start, void* end);
void mm_free_user_space(pcb_t* pcb);
void mm_free_address_space(pcb_t* pcb, boolean_t last_thread);

/** Requests information **/
int mm_getflags(pcb_t* pcb, void* addr);

/* FIXME mm_validate_write should be deleted!!! */
boolean_t mm_validate_write(void* addr, int len);

#endif /* end of include guard: MM_1PZ6H5QE */

