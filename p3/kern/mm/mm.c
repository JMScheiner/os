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
* @bug Kernel page allocation can probably do better than malloc.
* @bug We hold on to directory locks for a long time.
*/

#include <mm.h>
#include <mm_internal.h>   
#include <page.h>          // PAGE_SIZE, PAGE_SHIFT
#include <cr.h>            // get_cr3
#include <mutex.h>
#include <simics.h>
#include <assert.h>

#include <common_kern.h>
#include <string.h>        // memcpy, memset

/* Protects the free list. */
static mutex_t mm_lock;

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

   /* Explicitly unmap NULL to prevent NULL dereferences in the kernel. */
   pt = global_dir[0];
   pt[0] = 0; /* Not present, readable, or global */
   
   assert(addr == USER_MEM_START);
   assert(i == (USER_MEM_START >> DIR_SHIFT));
   
   /* Everything else is not present, so doesn't matter. */
   for( ; i < DIR_SIZE; i++) global_dir[i] = 0;

   mutex_init(&mm_lock);

   /* After this point we give up our direct access to pages in user land.*/
   set_cr3((uint32_t)global_dir);
   set_cr0(get_cr0() | CR0_PG);

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
   lprintf("Allocating new directory at %p", dir);

   /* The kernel part of every directory should be the same. */
   memcpy(dir, global_dir, (USER_MEM_START >> DIR_SHIFT) * sizeof(page_tablent_t*));
   
   /* Zero out everything else. */
   for(i = (USER_MEM_START >> DIR_SHIFT); i < DIR_SIZE; i++) dir[i] = 0;
   
   return (void*)dir;
}

/** 
* @brief Duplicates the current address space in the process
*  indicated by pcb. 
*
*  TODO We also need to copy the region list!   
*     - if page faults are expected to work correctly.
* 
* @param pcb The new process to copy into. The page directory 
*  should be empty, but allocated.  
*/
void mm_duplicate_address_space(pcb_t* pcb) 
{
   unsigned long d_index;
   unsigned long t_index;
   unsigned long flags;
   unsigned long new_page;

   page_dirent_t *new_dir, *current_dir;
   page_tablent_t *null_table, *current_table, *new_table;
   page_tablent_t current_frame, new_frame;
   free_block_t *free_block;
   
   /* More assertions! */
   assert(pcb->page_directory);
   
   new_dir = (page_dirent_t*)pcb->page_directory;
   current_dir = (page_dirent_t*)get_cr3();
   null_table = current_dir[0];
   
   /* Iterate over all frames in user space. */
   for(d_index = USER_MEM_START >> DIR_SHIFT; d_index < DIR_SIZE; d_index++)
   {
      current_table = current_dir[d_index];
      
      /* Skip nonexistent page tables. */
      if(!((unsigned long)current_table & PDENT_PRESENT)){
         continue;
      }

      /* Grab the flags from the current directory entry. */
      flags = (unsigned long)current_table & PAGE_MASK;
      current_table = (page_tablent_t*)PAGE_OF(current_table);

      /* Allocate a new table and set the appropriate flags. */
      new_table = (page_tablent_t*) mm_new_table();
      new_dir[d_index] = (page_tablent_t*)((int)new_table | flags);

      for(t_index = 0; t_index < TABLE_SIZE; t_index++)
      {
         current_frame = current_table[t_index];

         /* Skip nonexistent frames. */
         if(!(current_frame & PTENT_PRESENT)){
            continue;
         }

         /* Grab the flags from the current page table entry. */
         flags = current_frame & PAGE_MASK;
         new_page = (d_index << DIR_SHIFT) + (t_index << TABLE_SHIFT);

         /* Since NULL is inaccessible due to our defensive programming hack, 
          *    I am using it to map the frame in the new process. 
          *    While this is going on, accidental NULL dereferences will hose us...
          *    But there shouldn't be any of those. :D
          **/

         /* 
          *    1. Allocate a new frame to copy to. 
          *    2. Map it in the current address space. 
          *    3. Copy to it from the appropriate page. 
          *    4. Unmap it in the current address space. 
          *    5. Map it in the new address space. 
          */
         
         lprintf("Duplicating 0x%lx in new address space.", new_page);
         mutex_lock(&mm_lock);
         new_frame = (page_tablent_t) user_free_list;
         null_table[ 0 ] = ((unsigned long) new_frame | PTENT_PRESENT | PTENT_RW) ;
         invalidate_page((void*)0);

         /* We can use NULL to access the node now. */
         free_block = (free_block_t*)0;
         user_free_list = free_block->next;
         mutex_unlock(&mm_lock);

         /* Copy to the new frame (at NULL) */
         lprintf("....Copying");
         memcpy(NULL, (void*)new_page, PAGE_SIZE);
         
         /* Map the frame in the new process with the same flags as the current one.. */
         lprintf("....Mapping");
         new_table[t_index] = new_frame | flags;
      }
   }
   
   /* Unmap NULL for good measure. */
   null_table[ 0 ] = 0;
   invalidate_page((void*)0);

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
   
   return (void*)table;
}

/** 
* @brief Make the address "addr" available to the current thread
*  with flags "flags" for len bytes. 
*
*  The page will be filled with zeros initially.
*
*  Pages that already belong to the user will be skipped, and the 
*     "flags" value WILL NOT be applied.
* 
* @param addr The address to allocate.
* @param len Length of the new region.
* @param flags Flags for the region, e.g. PTENT_PRESENT
* 
* @return 
*/
void mm_alloc(pcb_t* pcb, void* addr, size_t len, unsigned int flags)
{
   assert(len > 0);
   assert((unsigned long)addr >= USER_MEM_START);

   /* Grab the current page directory. */
   page_dirent_t* dir = (page_dirent_t*) get_cr3();
   page_tablent_t* table;
   free_block_t* free_block;

   unsigned int page = PAGE_OF(addr);
   unsigned int npages = (PAGE_OF((unsigned long)addr + len - 1) - PAGE_OF(addr)) / PAGE_SIZE + 1;
   int i;
   
   mutex_lock(&pcb->mm_lock);
   for (i = 0; i < npages; i++, page += PAGE_SIZE) 
   {
      table = dir[ DIR_OFFSET(page) ];

      if(!((unsigned long)table & PTENT_PRESENT))
      {
         table = (page_tablent_t*) mm_new_table();
         dir[DIR_OFFSET(page)] = 
            (page_tablent_t*)((int)table | PDENT_PRESENT | flags);
      }
      else
      {
         /* Wipe the flags. */
         table = (page_tablent_t*)PAGE_OF(table);
      }
   
      /* SKIP Pages that are already allocated to us. 
       *    - The assumption here is that this function will be called 
       *      in order of priority, and that flags set by previous users will 
       *      be correct. To change flags use mm_setflags(addr, flags)
       */
      if(table[ TABLE_OFFSET(page) ] & PTENT_PRESENT)
      {
         continue;
      }

      /* Allocate the free page, but keep it in supervisor mode for now. */
      mutex_lock(&mm_lock);
      
      table[ TABLE_OFFSET(page) ] = 
         ((unsigned long) user_free_list | PTENT_PRESENT | flags) & ~PTENT_USER;
      invalidate_page((void*)page);
      
      /* We can use "page" to access the node now.*/
      free_block = (free_block_t*)page;
      user_free_list = free_block->next;

      mutex_unlock(&mm_lock);

      /* Wipe the free block part of the page. The rest should already be wiped. */
      memset((void*)page, 0, sizeof(free_block_t));
      
      /* Let the user see the page if appropriate. */
      if(flags & PTENT_USER)
      {
         table[ TABLE_OFFSET(page) ] = 
            ((unsigned long) user_free_list | PTENT_PRESENT | flags);
         invalidate_page((void*)page);
      }
   }
   mutex_unlock(&pcb->mm_lock);
}

/** 
* @brief Removes pages from the currently running processes address space. 
* 
* @param addr The page aligned address to free. 
* @param n The number of pages to remove. 
*/
void mm_free_pages(pcb_t* pcb, void* addr, size_t n)
{
   unsigned long page;
   page_dirent_t* dir;
   page_tablent_t* table;
   page_tablent_t frame;
   free_block_t* free_frame;
   
   assert(((unsigned int)addr & PAGE_MASK) == 0);
   assert((unsigned int)addr > USER_MEM_START);
   dir = (page_dirent_t*) get_cr3();

   mutex_lock(&pcb->mm_lock);
   for(page = (unsigned long) addr; page += PAGE_SIZE; n--)
   {
      table = dir[ DIR_OFFSET(page) ]; 
      
      /* Skip page tables that are not present. */
      if(!((unsigned long)table & PDENT_PRESENT)) continue;
   
      table = (page_tablent_t*)PAGE_OF(table);
      frame = table[ TABLE_OFFSET(page) ];
      
      if(!(frame & PTENT_PRESENT)) continue;
      
      /* Prevent other user space threads from touching the frame from here on. */
      table[ TABLE_OFFSET(page) ] = frame & ~PTENT_USER;
      invalidate_page((void*)page);
      
      /* Manage the newly freed frame. Note that we hold a lock to the 
       *    entire directory while we free this frame. This should only
       *    affect people who are freeing or allocating memory in this 
       *    process. 
       **/
      memset((void*)page, 0, PAGE_SIZE);
      free_frame = (free_block_t*) page;
     
      mutex_lock(&mm_lock);
      free_frame->next = user_free_list;
      user_free_list = (free_block_t*)frame;
      mutex_unlock(&mm_lock);

      /* This frame should now be invisible to this process. */
      table[ TABLE_OFFSET(page) ] = 0;
      invalidate_page((void*)page);

   }
   mutex_unlock(&pcb->mm_lock);
}

/** 
* @brief Returns the flags for the page "addr" is in.
* 
* @param addr The address to get the flags for. 
* 
* @return The flags (page directory and table bits 0).
*         Or -1 if the page directory entry is not present.
*/
int mm_getflags(void* addr)
{
   unsigned long page;
   long dflags, tflags;

   page = ((unsigned long)addr) & ~PAGE_MASK;
   page_dirent_t* dir = (page_dirent_t*) get_cr3();
   page_tablent_t* table = dir[ DIR_OFFSET(page) ];
   
   dflags = ((unsigned long)table) & PAGE_MASK;
   table = (page_tablent_t*)PAGE_OF(table);
   
   if(dflags & PDENT_PRESENT)
   {
      tflags = (unsigned long)table[ TABLE_OFFSET(page) ]  & PAGE_MASK;
      return tflags;
   }
   else return -1;
}

/** 
* @brief Used specifically to validate user arguments in system calls. 
*  For TRUE the page must be 
*     - Present
*     - Readable
*     - User mode.
* 
* @param addr 
* 
* @return 
*/
boolean_t mm_validate(void* addr)
{
   int tflags = mm_getflags(addr);
   if(tflags > 0 && ( tflags & (PTENT_USER | PTENT_PRESENT)))
      return TRUE;
   else return FALSE; 
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

