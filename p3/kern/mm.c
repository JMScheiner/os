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
* @bug Kernel pages allocation probably can do better than malloc.
* @bug Zeroing out pages probably happens at the wrong time. 
*/

#include <mm.h>
#include <mm_internal.h>   
#include <page.h>          // PAGE_SIZE, PAGE_SHIFT
#include <cr.h>            // get_cr3
#include <simics.h>
#include <assert.h>

#include <common_kern.h>
#include <string.h>        // memcpy, memset


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
   uint32_t addr = 0;
   
   page_tablent_t* pt;
   n_phys_frames = machine_phys_frames();
   free_block_t* iter;
   
   /* Point the free list at the first free page. */
   user_free_list = (free_block_t*)USER_MEM_START;

   /* This makes an assumption that initially memory is contiguous. */
   n_free_user_frames = n_phys_frames - (USER_MEM_START >> PAGE_SHIFT);

   /* Build a very simple link structure on free frames. */
   for(i = 0, iter = user_free_list; i < n_free_user_frames; i++, iter = iter->next)
      iter->next = (free_block_t*)((char*)iter + PAGE_SIZE);
   
   /* Initialize global page directory. */
   global_dir = (page_dirent_t*)mm_new_kernel_page();
   
   /* Iterate over directory entries in the kernel. */
   for(i = 0; i < (USER_MEM_START >> DIR_SHIFT); i++)
   {
      /* Allocate a page table for each. */
      pt = global_dir[i] = (page_tablent_t*) mm_new_kernel_page();

      /* Iterate over table entries. */
      for(j = 0; j < (TABLE_SIZE); j++, addr += PAGE_SIZE)
         pt[j] = addr | (PTENT_GLOBAL | PTENT_RW | PTENT_PRESENT);

      global_dir[i] = (page_dirent_t)((uint32_t)global_dir[i] | 
         (PDENT_GLOBAL | PDENT_RW | PDENT_PRESENT) );
   }
   
   assert(addr == USER_MEM_START);
   assert(i == (USER_MEM_START >> DIR_SHIFT));
   
   /* Everything else is not present, so doesn't matter. */
   for( ; i < DIR_SIZE; i++) global_dir[i] = 0;

   /* After this point we give up our direct access to pages in user land.*/
   set_cr3((uint32_t)global_dir);
   lprintf("Here we go....");
   set_cr0(get_cr0() | CR0_PG);
   lprintf("OKAY! VM is enabled!");

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
void* mm_new_directory()
{
   int i;
   
   /* Allocate the new page directory. */
   page_dirent_t* dir = (page_dirent_t*) mm_new_kernel_page();

   /* The kernel part of every directory should be the same. */
   memcpy(dir, global_dir, (USER_MEM_START >> DIR_SHIFT) * sizeof(page_tablent_t*));
   
   /* Zero out everything else. */
   for(i = (USER_MEM_START >> DIR_SHIFT); i < DIR_SIZE; i++) dir[i] = 0;
   
   return (void*)dir;
}

/** 
* @brief Allocates an empty page. 
*  
* @return the newly allocated table. 
*/
void* mm_new_table()
{
   int i;
   page_tablent_t* table = (page_tablent_t*)mm_new_kernel_page();
   for(i = 0; i < TABLE_SIZE; i++)
      table[i] = 0;
   
   //TODO What else needs to be done? 
   
   return (void*)table;
}

/** 
* @brief Allocates some number of physical pages at the given 
*  virtual address, in the current page table. 
*
*  TODO Need to be able to specify RW and user/supervisor.
* 
* @param n The number of pages to allocate.
* 
* @return 0 on success. 
*/
int mm_new_pages(void* addr, size_t n, unsigned int flags)
{
   assert(!((uint32_t)addr & PAGE_MASK));
   
   /* Grab the current page directory. */
   page_dirent_t* dir = (page_dirent_t*) get_cr3();
   page_tablent_t* table;
   free_block_t* free_block;
   
   assert((uint32_t)addr >= USER_MEM_START);
   assert(n > 0);
   assert(n_free_user_frames > n);

   while(n > 0)
   {
      table = dir[ DIR_OFFSET(addr) ];
      
      if(!((uint32_t)table & PTENT_PRESENT))
      {
         table = (page_tablent_t*)mm_new_table();
         dir[DIR_OFFSET(addr)] = (page_tablent_t*)((int)table | PDENT_PRESENT | flags);
      }
      else 
      {
         table = (page_tablent_t*)PAGE_OF(table);
      }
      
      /* We can't allocate a page that is already mapped */
      assert(! (table[ TABLE_OFFSET(addr) ] & PTENT_PRESENT) ); 
      
      table[ TABLE_OFFSET(addr) ] = (uint32_t)user_free_list | (PTENT_PRESENT | flags);
      invalidate_page(addr);
   
      /* FIXME Possible complication - 
       *  If we context switch here, we could pass execution to a thread
       *  which now has access to memory that is not zeroed out. 
       */ 

      /* We can use addr to access the node now.*/
      free_block = (free_block_t*)addr;
      
      /* Just point user_free_list at the next free frame. */
      user_free_list = free_block->next;
      memset(addr, 0, PAGE_SIZE);
      
      /* Move on to the next page. */
      n--; 
      addr += (PAGE_SIZE);
   }

   return 0;
}

int mm_alloc(void* addr, size_t len, unsigned int flags)
{
   /* Grab the current page directory. */
   page_dirent_t* dir = (page_dirent_t*) get_cr3();
   page_tablent_t* table;

   unsigned int page = (unsigned int)addr & (~PAGE_MASK); 
   unsigned int npages = (len + PAGE_SIZE - 1) / PAGE_SIZE;
   int i;
   
   for (i = 0; i < npages; i++, page += PAGE_SIZE) 
   {
      table = dir[ DIR_OFFSET(page) ];
      
      if(!((unsigned int)table & PTENT_PRESENT))
         mm_new_pages((void*)page, 1, flags);
      else 
         table = (page_tablent_t*)PAGE_OF(table);
      
      if(!(table[ TABLE_OFFSET(page) ] & PTENT_PRESENT))
         mm_new_pages((void*)page, 1, flags);
   }
   return 0;
}

/* TODO This definitely isn't the best way of allocating kernel pages. */

void* mm_new_kernel_pages(size_t n)
{
   void* addr = smemalign(PAGE_SIZE, n * PAGE_SIZE);
   assert(addr);
   return addr;
}

void* mm_new_kernel_page()
{
   void* addr = smemalign(PAGE_SIZE, PAGE_SIZE);
   assert(addr);
   return addr;
}

