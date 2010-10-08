/** 
* @file mm.c
* @brief Memory management.
*  
*  My assumptions so far:
*     - Free physical pages will be direct mapped, but inaccessible to users.
*
*  Problem: 
*     - When we try to traverse the free list, the addresses are physical - 
*        but most of the time when we are running on some user threads 
*        page directory. I need to sleep on it, but I think that turning off VM is 
*        one of the "bad things" 
*
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-10-07
*/

#include <mm.h>
#include <mm_internal.h>   
#include <page.h>          // PAGE_SIZE, PAGE_SHIFT
#include <cr.h>            // get_cr3
#include <assert.h>

#include <common_kern.h>
#include <string.h>        // memcpy




/** 
* @brief Initialize the user frame list. 
*  This function should only be called from kernel_main,
*     before interrupts are enabled.
* 
* @return 0 on success.
*/
int mm_init(void)
{
   int i, j;
   int32_t addr = 0;
   
   page_tablent_t* pt;
   n_phys_frames = machine_phys_frames();
   
   /* Point the free list at the first free page. */
   user_free_list = (free_block_t*)USER_MEM_START;

   /* This makes an assumption that initially memory is contiguous. */
   user_free_list->nframes = n_phys_frames - (USER_MEM_START >> PAGE_SHIFT);
   user_free_list->next = NULL;
   
   /* Initialize global page directory. */
   global_dir = (page_dirent_t*)mm_alloc_kernel_page();
   
   /* Iterate over directory entries in the kernel. */
   for(i = 0; i < USER_MEM_START >> DIR_SHIFT; i++)
   {
      /* Allocate a page table for each. */
      pt = global_dir[i] = (page_tablent_t*) mm_alloc_kernel_page();

      /* Iterate over table entries. */
      for(j = 0; j < (TABLE_SIZE >> 2); j++, addr += PAGE_SIZE)
         pt[j] = addr | (PTENT_GLOBAL | PTENT_RW | PTENT_PRESENT);

      global_dir[i] = (page_dirent_t)((int32_t)global_dir[i] | 
         (PDENT_GLOBAL | PDENT_RW | PDENT_PRESENT) );
   }

   assert(addr == USER_MEM_START);
   assert(i == (USER_MEM_START >> DIR_SHIFT));
   
   /* Everything else is not present, so doesn't matter. */
   for( ; i < DIR_SIZE; i++) global_dir[i] = 0;

   return 0;
}

/** 
* @brief Allocates a new empty page directory. 
*
*  The directory is initialized with: 
*     - Kernel pages direct mapped and present.
*     - Supervisory mode everywhere.
*     - Read only / not present in user land.
* 
* @return The address of the new directory in kernel space.
*/
void* mm_alloc_directory()
{
   int i;
   
   /* Allocate the new page directory. */
   page_dirent_t* dir = (page_dirent_t*) mm_alloc_kernel_page();

   /* The kernel part of every directory should be the same. */
   memcpy(dir, global_dir, (USER_MEM_START >> DIR_SHIFT) * sizeof(page_tablent_t*));
   
   /* Zero out everything else. */
   for(i = (USER_MEM_START >> DIR_SHIFT); i < DIR_SIZE; i++) dir[i] = 0;
   
   assert(i == (USER_MEM_START >> DIR_SHIFT));

   return (void*)dir;
}

/** 
* @brief Allocates an empty page. 
*  
* @return the newly allocated table. 
*/
void* mm_alloc_table()
{
   page_tablent_t* table = (page_tablent_t*)mm_alloc_kernel_page();
   
   //TODO What else needs to be done? 
   
   return (void*)table;
}

/** 
* @brief Allocates some number of physical pages at the given 
*  virtual address, in the current page table. 
* 
* @param n The number of pages to allocate.
* 
* @return 
*/
void* mm_new_pages(void* addr, size_t n)
{
   /* Grab the current page directory. */
   //page_dirent_t* dir = (page_dirent_t*) get_cr3();
   
   //TODO 
   
   return NULL;
}

/* TODO This definitely isn't the best way of allocating kernel pages. */

void* mm_alloc_kernel_pages(size_t n)
{
   return smemalign(PAGE_SIZE, n * PAGE_SIZE);
}

void* mm_alloc_kernel_page()
{
   return smemalign(PAGE_SIZE, PAGE_SIZE);
}

