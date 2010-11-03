/** 
* @file kvm.c
*
* @brief Implementation of kernel virtual memory. 
*  - All KVM tables are global, shared, and direct mapped. 
*  - There are at most (USER_MEM_END >> DIR_SHIFT) of these tables. 
*  - Not direct mapping the kvm tables was too much for
*     my puny brain to handle.
*  
* @author Justin Scheiner
* @author Tim Wilson
*/

#include <kvm.h>
#include <mm.h>
#include <mm_internal.h>
#include <kernel_types.h>
#include <mutex.h>
#include <global_thread.h>
#include <debug.h>
#include <simics.h>
#include <common_kern.h>
#include <string.h>

static void* _kvm_initial_table;
inline void* kvm_initial_table() { return _kvm_initial_table; }

/* A list of pages (virtual) that can be allocated.
 *    They are already mapped in the global directory.*/
static void* kvm_bottom;
static free_block_t* kernel_free_list;
static mutex_t kernel_free_lock;
static mutex_t new_table_lock;

static void* kvm_alloc_page(void* page);

/** 
* @brief Responsible for allocating the first KVM table and mapping it in 
*  the global directory. 
*/
void kvm_init()
{
   page_dirent_t* global_dir;
   _kvm_initial_table = mm_new_kp_page();
   
   kernel_free_list = NULL;
   kvm_bottom = KVM_TOP;
   mutex_init(&kernel_free_lock);
   mutex_init(&new_table_lock);
   
   global_dir = (page_dirent_t*)global_pcb()->dir_v;
   global_dir[ DIR_OFFSET(KVM_TOP) ] = (page_dirent_t)
      ((int)_kvm_initial_table | PDENT_PRESENT | PDENT_RW | PDENT_GLOBAL);
}

/** 
* @brief Frames "page" and returns the physical address of the frame. 
*
*  If the address requires a new global table to be allocated, it is,
*     and the new mapping for the table is reflected in _every_
*     process (ever).
* 
* @param page The page to frame. 
* 
* @return The physical address of the framed page. 
*/
void* kvm_alloc_page(void* page)
{
   unsigned long frame;
   
   assert(PAGE_OFFSET(page) == 0);
   assert((unsigned long)page >= USER_MEM_END);
   
   /* KVM tables are direct mapped. */
   page_dirent_t* dir;
   page_tablent_t* table;
   
   dir = (page_dirent_t*)global_pcb()->dir_v;
   table = dir[ DIR_OFFSET(page) ];
   
   if(!(FLAGS_OF(table) & PTENT_PRESENT))
   {
      mutex_lock(&new_table_lock);

      /* If, after locking the table still isn't allocated, 
       *  we are free to allocate it ourselves. */
      table = dir[ DIR_OFFSET(page) ];
      if(!(FLAGS_OF(table) & PTENT_PRESENT))
         table = kvm_new_table(page);
      else
         table = (page_tablent_t*)PAGE_OF(table);
      
      mutex_unlock(&new_table_lock);
   }
   else
   {
      table = (page_tablent_t*)PAGE_OF(table);
   }
   
   /* FIXME Maybe this should never happen... */
   if(FLAGS_OF(table[ TABLE_OFFSET(page) ]) & PTENT_PRESENT) 
      return (void*)PAGE_OF(table[ TABLE_OFFSET(page) ]) ;
   
   debug_print("kvm", "Mapping %p in table %p", page, table);
   frame = mm_new_frame((unsigned long*)table, (unsigned long)page);

   /* Set appropriate flags for the new frame. */
   table[ TABLE_OFFSET(page) ] = 
      ((unsigned long) frame | PTENT_PRESENT | PTENT_RW | PTENT_GLOBAL);
   invalidate_page((void*)page);
   return (void*)frame;
}

/** 
* @brief Allocates a new kernel virtual page.  
*  Since the tables are all shared, we use the global pcb. 
* 
* @return The virtual address of the framed page above USER_MEM_END
*/
void* kvm_new_page()
{
   void* new_page;
   
   mutex_lock(&kernel_free_lock);
   if(kernel_free_list)
   {
      assert((void*)kernel_free_list > (void*)USER_MEM_END);
      new_page = kernel_free_list;
      kernel_free_list = kernel_free_list->next;

      if((kernel_free_list != 0) && !((void*)kernel_free_list > (void*)USER_MEM_END))
         MAGIC_BREAK;

      mutex_unlock(&kernel_free_lock);
      
      //page_dirent_t* global_dir = global_pcb()->dir_v;
      //page_tablent_t* table = global_dir[ DIR_OFFSET(new_page) ];
      
      /* FIXME FIXME FIXME WHY DON'T I WORK. Remap the page. */
      /*table[ TABLE_OFFSET(new_page) ] = PAGE_OF(table[ TABLE_OFFSET(new_page) ])
         | PTENT_GLOBAL | PTENT_RW | PTENT_PRESENT;
      lprintf("Mapped page %p to %p", new_page, table[ TABLE_OFFSET(new_page) ]);*/
      invalidate_page(new_page);
   
   }
   else
   {
      new_page = kvm_bottom = kvm_bottom - PAGE_SIZE;
      
      /* When we fail this assertion it's time to do something smarter. */
      assert(kvm_bottom > (void*)USER_MEM_END);
      mutex_unlock(&kernel_free_lock);
      
      /* Frame the new page */
      kvm_alloc_page(new_page);
   }
   return new_page;
}

/** 
* @brief Makes "page" available to subsequent calls to 
*  kvm_new_page. 
* 
* @param page The kvm page to free. 
*/
void kvm_free_page(void* page)
{
   void* next;
   assert(page > (void*)USER_MEM_END);

   if(kernel_free_list)
      assert((void*)kernel_free_list > (void*)USER_MEM_END);

   //page_dirent_t* global_dir = global_pcb()->dir_v;
   /* Will there be race conditions on freed kernel pages? (TODO No) */
   //page_tablent_t* table = global_dir[ DIR_OFFSET(page) ];
   
   mutex_lock(&kernel_free_lock);
   
   lprintf("Adding page %p to kernel_free_list=%p", page, kernel_free_list);
   next = kernel_free_list;
   kernel_free_list = page;
   kernel_free_list->next = next;
   
   mutex_unlock(&kernel_free_lock);
   
   /* Wipe all flags, unmap the page. */
   //table[ TABLE_OFFSET(page) ] = PAGE_OF(table[ TABLE_OFFSET(page) ]);
   //invalidate_page(page);
   
   /* Is this all that needs to be done? */
   assert((void*)kernel_free_list > (void*)USER_MEM_END);
}

/** 
* @brief Allocates a new direct mapped, global kvm table. 
* 
* @param page The address to map the new table for - since we are
*  responsible for filling in every processes page directory.
* 
* @return The physical / virtual address of the new table. 
*/
void* kvm_new_table(void* addr)
{
   pcb_t *global, *iter;
   page_dirent_t* dir_v;
   
   global = global_pcb();
   dir_v = global->dir_v;
   assert(!(FLAGS_OF(dir_v[ DIR_OFFSET(addr) ]) & PTENT_PRESENT));

   /* kvm tables are direct mapped. */
   void* table = mm_new_kp_page();

   /* Need to update ALL directories. Luckily this only happens when the 
    *  kernel uses up 4M of space. */
   
   mutex_t* global_lock = global_list_lock();
   mutex_lock(global_lock);

   /* Map the new table in the global directory. */
   dir_v[ DIR_OFFSET(addr) ] = 
      (page_tablent_t*)((int)table | PDENT_GLOBAL | PDENT_PRESENT | PDENT_RW);
   
   /* Map the new table in every other PCB*/
   for (iter = global; (iter = LIST_NEXT(iter, global_node)) != global; )
   {
      lprintf("UPDATING GLOBAL TABLE, pid = %x", iter->pid);
      dir_v = iter->dir_v;
      dir_v[ DIR_OFFSET(addr) ] = 
         (page_tablent_t*)((int)table | PDENT_GLOBAL | PDENT_PRESENT | PDENT_RW);
   }
   
   mutex_unlock(global_lock);
   
   lprintf("Returning table at %p", table);
   return table;
}

/** 
* @brief Translates virtual to physical for addresses in 
*  kernel virtual memory. 
* 
* @param vaddr The virtual address translate. 
* 
* @return The virtual address, or NULL if the page isn't framed.
*/
void* kvm_vtop(void* vaddr)
{
   void* paddr;
   assert(vaddr > (void*)USER_MEM_END);
   
   page_dirent_t* dir = (page_dirent_t*)global_pcb()->dir_v;
   debug_print("kvm", "kvm_vtop: dir = %p", dir);
   page_tablent_t* table = dir[ DIR_OFFSET(vaddr) ];
   
   assert(FLAGS_OF(table) & PDENT_PRESENT);
   
   table = (page_tablent_t*)PAGE_OF(table);
   paddr = (void*)table[ TABLE_OFFSET(vaddr) ];
   
   assert(FLAGS_OF(paddr) & PTENT_PRESENT);
   
   return (void*)(PAGE_OF(paddr) + PAGE_OFFSET(vaddr));
}

/** 
* @brief Allocates a new initialized page directory in the given PCB.
* 
*  The directory is initialized with: 
*     - Kernel pages direct mapped and present.
*     - Supervisory mode everywhere.
*     - Read only / not present in user land.
*     - The directory itself mapped in kernel VM
*
*  The PCB will be updated with 
*     - the physical address of the new directory
*     - the virtual address of the new directory
*     - the virtual address of the virtual directory.
* 
*  We are also responsible for adding ourself to the global PCB list
*     that is used for managing the global kvm tables. 
* 
* @param pcb The PCB to endow with a new directory. 
*/
void kvm_new_directory(pcb_t* pcb)
{
   int i;
   page_dirent_t* global_dir = global_pcb()->dir_v;
   page_dirent_t* dir_v = kvm_new_page();
   page_dirent_t* virtual_dir_v = kvm_new_page();
   
   debug_print("mm", "Global directory at %p", global_dir);
   
   /* The global parts of every directory should be the same. */
   memset(dir_v, 0, PAGE_SIZE);
   memset(virtual_dir_v, 0, PAGE_SIZE);
   
   for(i = 0; i < (USER_MEM_START >> DIR_SHIFT); i++)
      virtual_dir_v[i] = (page_dirent_t)PAGE_OF(global_dir[i]);
   
   memcpy(dir_v, global_dir, 
      DIR_OFFSET(USER_MEM_START) * sizeof(page_tablent_t*));
   
   /* Need to acquire the new table lock here, otherwise a new global 
    *  table could get allocated, and we would never find out about it, 
    *  since we aren't on the global list. TODO Can the interaction of 
    *  the global and table locks deadlock? Which should we acquire first? */
   mutex_t* global_lock = global_list_lock();
   pcb_t* global = global_pcb();
   
   mutex_lock(&new_table_lock);
   
   /* When we do this copy the directory itself gets mapped as well. */
   for(i = DIR_OFFSET(kvm_bottom); i < DIR_SIZE; i++)
      virtual_dir_v[i] = (page_dirent_t)PAGE_OF(global_dir[i]);
  
   memcpy(dir_v + DIR_OFFSET(kvm_bottom), 
      global_dir + DIR_OFFSET(kvm_bottom), 
      (DIR_SIZE - DIR_OFFSET(kvm_bottom)) * sizeof(page_tablent_t*));
   
   /* Add ourselves to the global PCB list. */
   mutex_lock(global_lock);
   LIST_INSERT_AFTER(global, pcb, global_node); 
   mutex_unlock(global_lock);
   mutex_unlock(&new_table_lock);
   
   pcb->dir_v = dir_v;
   pcb->dir_p = kvm_vtop(dir_v);
   pcb->virtual_dir = virtual_dir_v;
}




