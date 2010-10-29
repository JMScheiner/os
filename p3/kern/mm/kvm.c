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

static void* _kvm_initial_table;
void* kvm_initial_table() { return _kvm_initial_table; }

/* A list of pages (virtual) that can be allocated.
 *    They are already mapped in the global directory.*/
static void* kvm_bottom;
static free_block_t* kernel_free_list;
static mutex_t kernel_free_lock;

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
      table = kvm_new_table(page);
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
      new_page = kernel_free_list;
      kernel_free_list = kernel_free_list->next;
      mutex_unlock(&kernel_free_lock);
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

   page_dirent_t* global_dir = global_pcb()->dir_v;
   
   /* Will there be race conditions on freed kernel pages? (TODO No) */
   page_tablent_t* table = global_dir[ DIR_OFFSET(page) ];
   table[ TABLE_OFFSET(page) ] = 
      PAGE_OF(table[ TABLE_OFFSET(page) ]);
   
   
   /* Is this all that needs to be done? */
   mutex_lock(&kernel_free_lock);
   
   next = kernel_free_list;
   kernel_free_list = page;
   kernel_free_list->next = next;
   
   mutex_unlock(&kernel_free_lock);
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
   /* kvm tables are direct mapped. */
   void* table = mm_new_kp_page();

   /* Need to update ALL directories. Luckily this shouldn't happen often. 
    *  Since the kernel should fill its existing tables fairly infrequently. */
   /*mutex_t* list_lock = pcb_list_lock();
   TODO TODO TODO TODO 
    Hold onto the kernel free lock while you do it. 
   mutex_lock(list_lock);

   FOREACH(global_queue, iter)
   {
      iter->dir_v[ DIR_OFFSET(addr) ]
         (page_tablent_t*)((int)table | PDENT_GLOBAL | PDENT_PRESENT | PDENT_RW);
   } 
   mutex_unlock(list_lock);*/

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



