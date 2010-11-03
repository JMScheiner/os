/** 
* @file mm.c
* @brief Memory management.
*  
* @author Justin Scheiner
* @author Tim Wilson
* @bug Kernel page allocation can probably do better than malloc.
* @bug We hold on to directory locks for a long time.
*/

#include <mm.h>
#include <kvm.h>
#include <mm_internal.h>   
#include <page.h>          // PAGE_SIZE, PAGE_SHIFT
#include <cr.h>            // get_cr3
#include <mutex.h>
#include <simics.h>
#include <region.h>
#include <assert.h>

#include <common_kern.h>
#include <string.h>        // memcpy, memset
#include <global_thread.h>
#include <debug.h>

#define COPY_PAGE ((void*)(-PAGE_SIZE))

static free_block_t* user_free_list;
static mutex_t user_free_lock;
static mutex_t copy_lock;

static void* mm_new_table(pcb_t* pcb, void* addr);
static void mm_free_table(pcb_t* pcb, void* addr);

/** 
* @brief Initialize the user frame list, enable paging.
*  This function should only be called from kernel_main,
*     before interrupts are enabled. It is also responsible 
*     adding the global directory to the global pcb.
* 
* @return 0 on success.
*/
int mm_init()
{
   int i, j;
   uint32_t addr = 0;
   
   page_tablent_t* table;
   page_dirent_t* global_dir;
   n_phys_frames = machine_phys_frames();
   free_block_t* iter;
   
   /* Point the free list at the first free page. */
   user_free_list = (free_block_t*)USER_MEM_START;

   /* This makes an assumption that initially memory is contiguous. */
   n_free_frames = n_phys_frames - (USER_MEM_START >> PAGE_SHIFT);

   /* Build a very simple link structure on free frames. */
   for(i = 0, iter = user_free_list; 
      i < n_free_frames - 1; 
      i++, iter = iter->next)
   {
      iter->next = (free_block_t*)((char*)iter + PAGE_SIZE);
   }
   iter->next = NULL;
   
   /* Initialize global page directory. V = P */
   global_pcb()->dir_v = global_pcb()->dir_p = (void*)mm_new_kp_page();
   global_dir = (page_dirent_t*)global_pcb()->dir_v;
   
   /* Iterate over directory entries in direct mapped kernel region. */
   for(i = 0; i < (USER_MEM_START >> DIR_SHIFT); i++)
   {
      /* Allocate a direct mapped page table for each. */
      table = global_dir[i] = (page_tablent_t*)mm_new_kp_page();
      
      global_dir[i] = (page_dirent_t)(
         (unsigned long)global_dir[i] | (PDENT_RW | PDENT_PRESENT) );

      /* Iterate over table entries. */
      for(j = 0; j < (TABLE_SIZE); j++, addr += PAGE_SIZE)
         table[j] = addr | (PTENT_GLOBAL | PTENT_RW | PTENT_PRESENT);
   }

   /* Explicitly unmap NULL to prevent NULL dereferences in the kernel. */
   table = (page_tablent_t*)PAGE_OF(global_dir[0]);
   table[0] = 0; /* Not present, writable, or global */
   
   assert(addr == USER_MEM_START);
   
   /* Everything else is not present, so doesn't matter. */
   for(; i < (USER_MEM_END >> DIR_SHIFT); i++) 
         global_dir[i] = 0;

   mutex_init(&user_free_lock);
   mutex_init(&copy_lock);
   
   /* Initialize kernel virtual memory which lives above 
    * USER_MEM_END and is global. */
   kvm_init();

   /* After this point we give up our direct access to pages in user land.*/
   set_cr3((uint32_t)global_dir);
   set_cr0(get_cr0() | CR0_PG);

   return 0;
}

/** 
* @brief Frees all frames allocated to the user. Handy for use in 
*  mm_free_address_space and exec. 
* 
* @param pcb The PCB to free the user space of.
*/
void mm_free_user_space(pcb_t* pcb)
{
   unsigned long d_index;
   unsigned long t_index;
   unsigned long frame, page;
   pcb_t* global;
   page_dirent_t* dir_v, *virtual_dir;
   page_tablent_t *table_v, *table_p;
   
   global = global_pcb();
   dir_v = pcb->dir_v;
   virtual_dir = pcb->virtual_dir;
   
   for(d_index = DIR_OFFSET(USER_MEM_START); 
      d_index < DIR_OFFSET(USER_MEM_END); d_index++)
   {
      table_v = virtual_dir[d_index];
      table_p = dir_v[d_index];

      if(!(FLAGS_OF(table_p) & PDENT_PRESENT)) continue;
      
      for(t_index = 0; t_index < TABLE_SIZE; t_index++)
      {
         frame = table_v[t_index];
         if(!(FLAGS_OF(frame) & PTENT_PRESENT)) continue;
         
         page = (d_index << DIR_SHIFT) + (t_index << TABLE_SHIFT);
         mm_free_frame(table_v, page);
      }
      mm_free_table(pcb, (void*)(d_index << DIR_SHIFT));
      dir_v[d_index] = 0;
   }
}

/** 
* @brief Frees every frame and table that belongs to user space. 
*  Releases the directories, and continues execution in the global directory.
* 
* @param pcb The process that points to the relevant address space. 
*/
void mm_free_address_space(pcb_t* pcb)
{
   pcb_t* global;
   page_dirent_t* dir_v, *virtual_dir;
   
   global = global_pcb();
   dir_v = pcb->dir_v;
   virtual_dir = pcb->virtual_dir;
   
   /* All other tables are global - we don't need to worry about them. */
   mm_free_user_space(pcb);
   
   pcb->dir_v = global->dir_v;
   pcb->dir_p = global->dir_p;
   pcb->virtual_dir = global->virtual_dir;
   set_cr3((int)pcb->dir_p);
   
   kvm_free_page(dir_v);
   kvm_free_page(virtual_dir);
}

/** 
* @brief Duplicates the current address space in the process
*  indicated by pcb. 
*
* @param pcb The new process to copy into. The page directory 
*  should be empty, but allocated.  
*/
void mm_duplicate_address_space(pcb_t* new_pcb) 
{
   unsigned long d_index;
   unsigned long t_index;
   unsigned long flags;
   unsigned long page;
   pcb_t* current_pcb;
   
   page_dirent_t *current_dir_v, *new_dir_v, *current_virtual_dir;

   page_tablent_t *copy_table_v, *current_table_v, *new_table_v, 
      *current_table_p;

   page_tablent_t current_frame, new_frame;
   
   current_pcb = get_pcb();
   current_dir_v = current_pcb->dir_v;
   new_dir_v = new_pcb->dir_v;
   current_virtual_dir = current_pcb->virtual_dir;

   copy_table_v = kvm_initial_table();

   for(d_index = (USER_MEM_START >> DIR_SHIFT); 
      d_index < (USER_MEM_END >> DIR_SHIFT); d_index++)
   {
      current_table_v = current_virtual_dir[d_index];
      current_table_p = current_dir_v[d_index];
      
      if(!(FLAGS_OF(current_table_p) & PDENT_PRESENT)) continue;
      
      flags = FLAGS_OF(current_table_p);
      
      /* Also maps the table in the new process's dir / virtual directory. */
      new_table_v = mm_new_table(new_pcb, (void*)(d_index << DIR_SHIFT));
      
      for(t_index = 0; t_index < TABLE_SIZE; t_index++)
      {
         current_frame = current_table_v[t_index];
         if(!(FLAGS_OF(current_frame) & PTENT_PRESENT)) continue;

         flags = FLAGS_OF(current_frame);
         page = (d_index << DIR_SHIFT) + (t_index << TABLE_SHIFT);
         
         debug_print("mm", "Copying page 0x%lx", page);
         
         new_frame = mm_new_frame((unsigned long*)copy_table_v, (unsigned long)COPY_PAGE);

         /* When this fails time to do something smarter. */
         assert(new_frame);


         memcpy((void*)COPY_PAGE, (void*)page, PAGE_SIZE);
         new_table_v[t_index] = new_frame | flags;
      }
   }
   
   /* Unmap the page we used to copy for good measure. */
   copy_table_v[ TABLE_OFFSET(COPY_PAGE) ] = 0;
   invalidate_page((void*)COPY_PAGE);
}

/** 
* @brief Allocates a page for a new table. 
*  1. Initializes all of it's entries as non-present.
*  2. Maps the table in the PCB's directories, at the address indicated.
*
*  @return The virtual address of the new table. 
*/
void* mm_new_table(pcb_t* pcb, void* addr)
{
   void *table_v, *table_p;
   
   page_dirent_t* dir_v = pcb->dir_v;
   page_dirent_t* virtual_dir_v = pcb->virtual_dir;
   
   table_v = kvm_new_page();
   table_p = kvm_vtop(table_v); 

   memset(table_v, 0, PAGE_SIZE);
   dir_v[ DIR_OFFSET(addr) ] = 
      (page_dirent_t)((unsigned long)table_p | PDENT_USER | PDENT_PRESENT | PDENT_RW);
   virtual_dir_v[ DIR_OFFSET(addr) ] = table_v;
   return table_v;
}

void mm_free_table(pcb_t* pcb, void* addr)
{
   void *table_v;
   
   page_dirent_t* dir_v = pcb->dir_v;
   page_dirent_t* virtual_dir_v = pcb->virtual_dir;
   table_v = virtual_dir_v[ DIR_OFFSET(addr) ];

   debug_print("mm", "About to free table %p for address %p", table_v, addr);
   kvm_free_page(table_v);
   
   virtual_dir_v[ DIR_OFFSET(addr) ] = 0;
   dir_v[ DIR_OFFSET(addr) ] = 0;
}

/** 
* @brief Allocates frames for the address range in the given processes 
*  address space, with flags indicated by "flags".
*
*  The pages will be filled with zeros initially.
*
*  Pages that already belong to the user will be skipped, and the 
*     "flags" value WILL NOT be applied.
* 
* @param addr The address to allocate.
* @param len Length of the new region in bytes.
* @param flags Flags for the region, e.g. PTENT_PRESENT
*/
void mm_alloc(pcb_t* pcb, void* addr, size_t len, unsigned int flags)
{
   assert(len > 0);
   assert((unsigned long)addr >= USER_MEM_START && 
      (unsigned long)addr < USER_MEM_END);

   page_dirent_t* dir_v = (page_dirent_t*)pcb->dir_v;
   page_dirent_t* virtual_dir = (page_dirent_t*)pcb->virtual_dir;
   page_tablent_t* table_p;
   page_tablent_t* table_v;

   unsigned int page = PAGE_OF(addr);
   unsigned int npages =   
      (PAGE_OF((unsigned long)addr + len - 1) - PAGE_OF(addr)) / PAGE_SIZE + 1;
   unsigned long frame;
   int i;
   
   mutex_lock(&pcb->directory_lock);
   for (i = 0; i < npages; i++, page += PAGE_SIZE) 
   {
      table_p = (page_tablent_t*)dir_v[ DIR_OFFSET(page) ];
      if( !(FLAGS_OF(table_p) & PTENT_PRESENT) )
         mm_new_table(pcb, (void*)page);

      table_v = (page_tablent_t*)virtual_dir[ DIR_OFFSET(page) ];
   
      /* SKIP Pages that are already allocated to us. 
       *    - The assumption here is that this function will be called 
       *      in order of priority, and that flags set by previous users will 
       *      be correct. To change flags use TODO mm_setflags(addr, flags)
       */
      if(FLAGS_OF(table_v[ TABLE_OFFSET(page) ]) & PTENT_PRESENT) 
         continue;

      /* Allocate the free page, but keep it in supervisor mode for now. */
      frame = mm_new_frame((unsigned long*)table_v, page);
      
      /* Reassign the page with the flags the user originally asked for. */
      debug_print("mm", "Mapping page 0x%x to frame 0x%lx with flags %x", page, frame, flags);
      debug_print("mm", "....in table %p", table_v);
      table_v[ TABLE_OFFSET(page) ] = 
         ((unsigned long) frame | PTENT_PRESENT | flags);
      invalidate_page((void*)page);
   }
   mutex_unlock(&pcb->directory_lock);
} 


/** 
* @brief Removes pages from the currently running processes address space. 
*
*  Note that this deliberately asks for the addresses to be page aligned. 
*     There isn't a great way of freeing non-page aligned memory, without
*     keeping track of what is "allocated" 
* 
* @param addr The page aligned address to free. 
* @param n The number of pages to remove. 
*/
void mm_free_pages(pcb_t* pcb, void* addr, size_t n)
{
   unsigned long page;
   page_dirent_t *dir_v, *virtual_dir_v;
   page_tablent_t *table_v, *table_p;
   
   assert(((unsigned int)addr & PAGE_MASK) == 0);
   assert((unsigned int)addr > USER_MEM_START);
   
   dir_v = (page_dirent_t*)pcb->dir_v;
   virtual_dir_v = (page_dirent_t*)pcb->virtual_dir;
   
   mutex_lock(&pcb->directory_lock);
   for(page = (unsigned long) addr; page += PAGE_SIZE; n--)
   {
      table_p = dir_v[ DIR_OFFSET(page) ]; 
      
      /* Skip page tables that are not present. */
      if(!(FLAGS_OF(table_p) & PDENT_PRESENT)) 
         continue;
   
      table_v = (page_tablent_t*)virtual_dir_v[ DIR_OFFSET(page) ];

      /* We let mm_free_frame silently return a -1 
       *    if the page isn't mapped */
      mm_free_frame(table_v, page);
   }
   mutex_unlock(&pcb->directory_lock);
}

/** 
* @brief Returns the flags for the page "addr" is in.
* 
* @param addr The address to get the flags for. 
* 
* @return The flags (page directory and table bits 0).
*         Or -1 if the page directory entry is not present.
*/
int mm_getflags(pcb_t* pcb, void* addr)
{
   unsigned long page;
   long dflags, tflags;

   page = PAGE_OF(addr);
   page_dirent_t* dir_v = (page_dirent_t*) pcb->dir_v;
   page_dirent_t* virtual_dir_v = (page_dirent_t*) pcb->virtual_dir;
   page_tablent_t* table_p = dir_v[ DIR_OFFSET(page) ];
   page_tablent_t* table_v = virtual_dir_v[ DIR_OFFSET(page) ];
   
   dflags = ((unsigned long)table_p) & PAGE_MASK;
   
   if(FLAGS_OF(table_p) & PDENT_PRESENT)
   {
      tflags = FLAGS_OF(table_v[ TABLE_OFFSET(page) ]);
      return tflags;
   }
   else return -1;
}

/** 
* @brief Used to validate a memory region for reading.
*     For TRUE every page in the region must be 
*     - Present
* 
* @param addr The address to validate. 
* 
* @return True if the address is safe.
*/
boolean_t mm_validate_read(void* addr, int len)
{
	unsigned int npages = NUM_PAGES(addr, len);
	int i;
	for (i = 0; i < npages; i++) {
		int tflags = mm_getflags(get_pcb(), (void*)addr + i*PAGE_SIZE);
		if(tflags <= 0 || 
				!TEST_SET(tflags, (PTENT_PRESENT)))
			return FALSE;
	}
	return TRUE;
}

/**
 * @brief Used to validate a memory region for writing.
*     For TRUE every page in the region must be 
 *        - Present
 *        - Writable
 *
 * @param addr The base address
 * @param len The number of bytes to check
 *
 * @return True if the address is safe.
 */
boolean_t mm_validate_write(void *addr, int len)
{
	unsigned int npages = NUM_PAGES(addr, len);
	int i;
	for (i = 0; i < npages; i++) {
		int tflags = mm_getflags(get_pcb(), (void*)addr + i*PAGE_SIZE);
		if(tflags <= 0 || 
				!TEST_SET(tflags, (PTENT_PRESENT | PTENT_RW)))
			return FALSE;
	}
	return TRUE;
}

/** 
* @brief Allocates a new kernel physical page. 
*  TODO Since we have stricter, consistent alignment requirements when we are 
*     calling this, it may be beneficial to take some memory away
*     from the kernel heap and replace it with our own frame allocator.
* 
* @return A framed, direct-mapped page below USER_MEM_START
*/
void* mm_new_kp_page()
{
   void* addr = smemalign(PAGE_SIZE, PAGE_SIZE);
   memset(addr, 0, PAGE_SIZE);

   /* When this assertion fails, do something more intelligent. */
   assert(addr);
   return addr;
}

/** 
* @brief Safely allocates a new frame.
*  1. Allocates a new frame for the current process.
*  2. Maps that frame to the place indicated by "page" in rw/supervisor mode.
*  3. Uses the new mapping to update the free list.
* 
* @param table The page table that "page" belongs to. 
* @param page The page to map. 
* 
* @return The physical address of the new frame.
*/
unsigned long mm_new_frame(unsigned long* table_v, unsigned long page)
{
   unsigned long new_frame;
   free_block_t* free_block;
   
   mutex_lock(&user_free_lock);
   new_frame = (unsigned long)user_free_list;
   
   /* When this assertion fails, do something more intelligent. */
   assert(new_frame);
   
   table_v[ TABLE_OFFSET(page) ] = new_frame | PTENT_PRESENT | PTENT_RW; 
   invalidate_page((void*)page);
   
   free_block = (free_block_t*)page;
   user_free_list = free_block->next;
   memset((void*)page, 0, sizeof(free_block_t));
   n_free_frames--;
   //lprintf("n_free_frame = %d", n_free_frames);

   mutex_unlock(&user_free_lock);
   return new_frame;
}

/** 
* @brief Releases a frame into the free frame pool.
* 
* @param table The page table the page occupies. 
* @param page The page to free from the address space associated with table. 
* 
* @return 0 on success, a negative integer on failure. 
*/
unsigned long mm_free_frame(unsigned long* table_v, unsigned long page)
{
   free_block_t* node;
   unsigned long frame;

   if(page == 0xfffce000) MAGIC_BREAK;
   
   frame = table_v[ TABLE_OFFSET(page) ];
   if(!(FLAGS_OF(frame) & PTENT_PRESENT)) return -1;
      
   frame = PAGE_OF(frame);
   
   /* Take the page away from user threads if necessary. */
   table_v[ TABLE_OFFSET(page) ] = frame | PTENT_PRESENT | PTENT_RW; 
   invalidate_page((void*)page);
   
   /* Clear the frame.*/
   memset((void*)page, 0, PAGE_SIZE);
   node = (free_block_t*) page;
  
   mutex_lock(&user_free_lock);
   node->next = user_free_list;
   user_free_list = (free_block_t*)frame;
   n_free_frames++;
   mutex_unlock(&user_free_lock);

   /* This frame should now be invisible to the process. */
   table_v[ TABLE_OFFSET(page) ] = 0;
   invalidate_page((void*)page);

   return 0;
}


