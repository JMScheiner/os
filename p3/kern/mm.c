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
#include <simics.h>
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
   uint32_t addr = 0;
   
   page_tablent_t* pt;
   n_phys_frames = machine_phys_frames();
   
   /* Point the free list at the first free page. */
   user_free_list = (free_block_t*)USER_MEM_START;

   /* This makes an assumption that initially memory is contiguous. */
   n_free_user_frames = user_free_list->nframes = n_phys_frames - (USER_MEM_START >> PAGE_SHIFT);
   user_free_list->next = NULL;
   
   /* Initialize global page directory. */
   global_dir = (page_dirent_t*)mm_new_kernel_page();
   
   /* Iterate over directory entries in the kernel. */
   for(i = 0; i < (USER_MEM_START >> DIR_SHIFT); i++)
   {
      /* Allocate a page table for each. */
      pt = global_dir[i] = (page_tablent_t*) mm_new_kernel_page();
      lprintf("New kernel page at %p", pt);

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
   
   assert(i == (USER_MEM_START >> DIR_SHIFT));

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
void* mm_new_pages(void* addr, size_t n)
{
   assert(!((uint32_t)addr & PAGE_MASK));
   
   /* Grab the current page directory. */
   page_dirent_t* dir = (page_dirent_t*) get_cr3();
   page_tablent_t* table;
   free_block_t* free_block, *next_block;


   uint32_t nframes, free_frame;

   assert((uint32_t)addr > USER_MEM_START);
   assert(n > 0);
   assert(n_free_user_frames > n);

   while(n > 0)
   {
      table = dir[ DIR_OFFSET(addr) ];
      if(!((uint32_t)table & PTENT_PRESENT))
      {
         table = (page_tablent_t*)mm_new_table();
      }
      else 
      {
         table = (page_tablent_t*)PAGE_OF(table);
      }
      
      /* We can't allocate a page that is already mapped */
      assert(! (table[ TABLE_OFFSET(addr) ] & PTENT_PRESENT) ); 
      
      table[ TABLE_OFFSET(addr) ] = (uint32_t)user_free_list | (PTENT_PRESENT | PTENT_RW);
      invalidate_page(addr);

      /* If things work the way I think they do, we should be able to use *addr to access
       *    the node now. 
       */
      free_block = (free_block_t*)addr;
      if(free_block->nframes == 1)
      {
         /* Just point user_free_list at the next free frame. */
         user_free_list = free_block->next;
      }
      else
      {
         /* Copy the information from this page to the next free page. */
         free_frame = (uint32_t) user_free_list;
         nframes = free_block->nframes;
         next_block = free_block->next;
         
         user_free_list = user_free_list + (PAGE_SIZE / sizeof(free_block_t*));
         
         /* We need to remap the frame to the new block in order to modify it. */
         table[ TABLE_OFFSET(addr) ] = (uint32_t)user_free_list | 
            (PTENT_PRESENT | PTENT_RW);
         
         invalidate_page(addr);
         
         free_block->nframes = nframes - 1;
         free_block->next = next_block;
         
         /* And then map it back to the correct frame. */
         table[ TABLE_OFFSET(addr) ] = (uint32_t) free_frame | 
            (PTENT_PRESENT | PTENT_RW | PTENT_USER);
         
         invalidate_page(addr);
      }
      
      /* Move on to the next page. */
      n--; 
      addr += (PAGE_SIZE / sizeof(void*));
   }

   return 0;
}

/* TODO This definitely isn't the best way of allocating kernel pages. */

void* mm_new_kernel_pages(size_t n)
{
   void* addr = smemalign(PAGE_SIZE, n * PAGE_SIZE);

   // NOTE: smemalign returned NULL on the first try. 
   //    It is not clear that this is the wrong behavior...
   //assert(addr);
   return addr;
}

void* mm_new_kernel_page()
{
   void* addr = smemalign(PAGE_SIZE, PAGE_SIZE);
   //assert(addr);
   return addr;
}

