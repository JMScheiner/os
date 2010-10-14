#ifndef MM_INTERNAL_DR6WBXWC

#define MM_INTERNAL_DR6WBXWC

#include <stdint.h>

#define DIR_SIZE 1024
#define TABLE_SIZE 1024
#define DIR_SHIFT 22
#define TABLE_SHIFT 12
#define OFFSET_MASK (DIR_SIZE - 1)

// Page directory entry flags.
#define PDENT_PRESENT         0x1
#define PDENT_RW              0x2    
#define PDENT_USER            0x4
#define PDENT_WRITE_THROUGH   0x8
#define PDENT_DISABLE_CACHE   0x10
#define PDENT_ACCESSED        0x20
#define PDENT_RESERVED        0x40
#define PDENT_2MPAGESIZE      0x80
#define PDENT_GLOBAL          0x100

// Page table entry flags. 
#define PTENT_WRITE_THROUGH   0x8
#define PTENT_DISABLE_CACHE   0x10
#define PTENT_ACCESSED        0x20
#define PTENT_DIRTY           0x40
#define PTENT_ATTR            0x80
#define PTENT_GLOBAL          0x100

#define DIR_OFFSET(addr) ((((int)addr) >> DIR_SHIFT) & OFFSET_MASK)
#define TABLE_OFFSET(addr) ((((int)addr) >> TABLE_SHIFT) & OFFSET_MASK)

/* @brief Local copy of the total number of physical frames in the system.
 *  mm implementation assumes contiguous memory. */
int n_phys_frames;
int n_free_user_frames;

typedef uint32_t page_tablent_t;
typedef page_tablent_t* page_dirent_t;

/** 
* @brief A single node in the free frame structure.
*/
typedef struct _FREE_BLOCK_T 
{
   /* @brief The next free block of physical memory. */
   struct _FREE_BLOCK_T* next;
} free_block_t;

free_block_t* user_free_list; 
page_dirent_t* global_dir;

void* mm_new_kernel_page(void);
void invalidate_page(void* addr);

#endif /* end of include guard: MM_INTERNAL_DR6WBXWC */

