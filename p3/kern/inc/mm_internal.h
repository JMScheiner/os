/** 
* @file mm_internal.h
*
* @brief Definitions for the internal memory management code. 
*
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-11-12
*/
#ifndef MM_INTERNAL_DR6WBXWC
#define MM_INTERNAL_DR6WBXWC

#include <kernel_types.h>

#define DEFAULT_COPY_PAGE ((void*)(USER_MEM_END))
#define FREE_PAGE ((void*)(-1 * PAGE_SIZE))

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

#define TABLE_PRESENT(table) ((unsigned long)(FLAGS_OF(table) & PDENT_PRESENT))
#define PAGE_PRESENT(page) ((unsigned long)(FLAGS_OF(page) & PTENT_PRESENT))
#define PAGE_FROM_INDEX(d, t) (((d) << DIR_SHIFT) + ((t) << TABLE_SHIFT))

/** 
* @brief A single node in a very simple free list. 
*/
typedef struct FREE_BLOCK 
{
   /* @brief The next free block of physical memory. */
   struct FREE_BLOCK* next;
} free_block_t;

typedef unsigned long page_tablent_t;
typedef page_tablent_t* page_dirent_t;

void invalidate_page(void* addr);
unsigned long mm_new_frame(unsigned long* table, unsigned long page);
unsigned long mm_free_frame(unsigned long* table, unsigned long page);
void* mm_new_table(pcb_t* pcb, void* addr);
void mm_free_table(pcb_t* pcb, void* addr);

#endif /* end of include guard: MM_INTERNAL_DR6WBXWC */

