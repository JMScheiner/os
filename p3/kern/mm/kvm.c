/** 
* @file kvm.c
*
* @brief Implementation of kernel virtual memory. 
*  - All kvm tables are global, shared, and direct mapped. 
*  - There are at most DIR_OFFSET(KVM_END - KVM_START) of these tables. 
*  - kvm tables are direct mapped. 
*
*  - kvm is NOT responsible for requesting frames. This should occur 
*    at the highest reasonable level. 
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
#include <ecodes.h>
#include <atomic.h>

static void* _kvm_initial_table;
inline void* kvm_initial_table() { return _kvm_initial_table; }


/* A list of pages (virtual) that can be allocated.
 *    They are already mapped in the global directory.*/
static void* kvm_bottom;
static free_block_t* kernel_free_list;
static mutex_t kernel_free_lock;
static mutex_t new_table_lock;

static void* kvm_alloc_page(void* page);
static void* kvm_new_table(void* addr);

static int n_kernel_frames;
static mutex_t kernel_request_lock;

/** 
* @brief Requests frames for allocation. 
* 
* @param n_user The number of user frames to allocate. 
* @param n_kernel The number of kernel frames to allocate. 
*/
int kvm_request_frames(int n_user, int n_kernel)
{
   int ret = ENOVM;
   mutex_lock(&kernel_request_lock);
   
   /* If there aren't enough kernel frames to satisfy the request, 
    *  we'll need to allocate from the general frame pool. */
   if((n_kernel_frames - n_kernel) < 0)
   {
      n_user += (n_kernel - n_kernel_frames);
      n_kernel = n_kernel_frames;
   }
   
   ret = mm_request_frames(n_user);
   if(ret == 0)
      n_kernel_frames -= n_kernel;

   mutex_unlock(&kernel_request_lock);
   return ret;
}

/** 
* @brief Responsible for allocating the first kvm table and mapping it in 
*  the global directory. 
*/
void kvm_init()
{
   page_dirent_t* global_dir;
   _kvm_initial_table = mm_new_kp_page();
   assert(_kvm_initial_table);
   
   kernel_free_list = NULL;
   kvm_bottom = KVM_END;
   mutex_init(&kernel_free_lock);
   mutex_init(&new_table_lock);
   mutex_init(&kernel_request_lock);
   n_kernel_frames = 0;
   
   global_dir = (page_dirent_t*)global_pcb()->dir_v;
   global_dir[ DIR_OFFSET(KVM_END) ] = (page_dirent_t)
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
*  Or NULL on failure. 
*/
void* kvm_alloc_page(void* page)
{
   unsigned long frame;
   
   assert(PAGE_OFFSET(page) == 0);
   assert((unsigned long)page >= USER_MEM_END);
   
   /* kvm tables are direct mapped. */
   page_dirent_t* dir;
   page_tablent_t* table;
   
   /* Note we may not have a stack yet, so it isn't safe to reference
    *  our TCB (and therefore our own PCB). */
   dir = (page_dirent_t*)get_pcb()->dir_v;
   table = dir[ DIR_OFFSET(page) ];
   
   /* Deal with new global table allocation if necessary. */
   if(!TABLE_PRESENT(table))
   {
      mutex_lock(&new_table_lock);

      /* If, after locking the table still isn't allocated, 
       *  we are free to allocate it ourselves. */
      table = dir[ DIR_OFFSET(page) ];
      if(!TABLE_PRESENT(table))
      {
         if((table = kvm_new_table(page)) == NULL)
            return NULL;
      }
      else
      {
         table = (page_tablent_t*)PAGE_OF(table);
      }
      
      mutex_unlock(&new_table_lock);
   }
   else 
      table = (page_tablent_t*)PAGE_OF(table);
   
   assert(FLAGS_OF(table) == 0);
   assert(!PAGE_PRESENT(table[ TABLE_OFFSET(page) ]));
   
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
* @return The virtual address of the framed page above KVM_START
*/
void* kvm_new_page()
{
   void* new_page;
   
   mutex_lock(&kernel_free_lock);
   if(kernel_free_list)
   {
      assert((void*)kernel_free_list > (void*)KVM_START);
      new_page = kernel_free_list;
      
      /* Remap the page. */
      page_dirent_t* global_dir = global_pcb()->dir_v;
      page_tablent_t* table = 
         (page_tablent_t*)PAGE_OF(global_dir[ DIR_OFFSET(new_page) ]);
      
      assert(FLAGS_OF(table) == 0);
      table[ TABLE_OFFSET(new_page) ] = PAGE_OF(table[ TABLE_OFFSET(new_page) ])
         | PTENT_GLOBAL | PTENT_RW | PTENT_PRESENT;
      invalidate_page(new_page);
      
      kernel_free_list = kernel_free_list->next;
      debug_print("kvm", " Allocated new page at %p, kernel_free_list = %p", 
         new_page, kernel_free_list);
      
      mutex_unlock(&kernel_free_lock);
   }
   else
   {
      new_page = kvm_bottom = kvm_bottom - PAGE_SIZE;
      
      /* When we fail this assertion it's time to do something smarter. */
      assert(kvm_bottom > (void*)KVM_START);
      mutex_unlock(&kernel_free_lock);
      
      /* Frame the new page */
      if(kvm_alloc_page(new_page) == NULL)
         return NULL;
   }
   memset(new_page, 0, PAGE_SIZE);
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
   assert(page > (void*)KVM_START);

   if(kernel_free_list)
      assert((void*)kernel_free_list > (void*)KVM_START);
   
   /* Clear the free frame list part of the recovered page. */
   memset(page, 0, sizeof(free_block_t));
   
   /* Will there be race conditions on freed kernel pages? (TODO No) */
   page_dirent_t* global_dir = global_pcb()->dir_v;
   page_tablent_t* table = global_dir[ DIR_OFFSET(page) ];
   table = (page_tablent_t*)PAGE_OF(table);
   
   mutex_lock(&kernel_request_lock);
   mutex_lock(&kernel_free_lock);
   
   debug_print("kvm", "Adding page %p to kernel_free_list=%p", page, kernel_free_list);
   next = kernel_free_list;
   kernel_free_list = page;
   kernel_free_list->next = next;
   
   n_kernel_frames++;
   
   assert((void*)kernel_free_list > (void*)KVM_START);
   
   /* Unmap the page to make illegal accesses show up in debugging. */
   assert(FLAGS_OF(table) == 0);
   table[ TABLE_OFFSET(page) ] = PAGE_OF(table[ TABLE_OFFSET(page) ]);
   invalidate_page(page);
   
   mutex_unlock(&kernel_free_lock);
   mutex_unlock(&kernel_request_lock);
   
   /* Wipe all flags, unmap the page. */
   
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
   void* table;
   pcb_t *global, *iter;
   page_dirent_t* dir_v;
   page_dirent_t* virtual_dir;
   
   global = global_pcb();
   dir_v = global->dir_v;
   assert(!TABLE_PRESENT(dir_v[ DIR_OFFSET(addr) ]));

   /* kvm tables are direct mapped. */
   if((table = mm_new_kp_page()) == NULL)
      return NULL;

   /* Need to update ALL directories. Luckily this only happens when the 
    *  kernel uses up 4M of space. */
   mutex_t* global_lock = global_list_lock();
   mutex_lock(global_lock);

   /* Map the new table in every other PCB*/
   LIST_FORALL(global, iter, global_node)
   {
      debug_print("kvm", "UPDATING GLOBAL TABLE, pid = %x", iter->pid);
      dir_v = iter->dir_v;
      virtual_dir = iter->virtual_dir;

      assert(FLAGS_OF(table) == 0);
      dir_v[ DIR_OFFSET(addr) ] = 
         (page_tablent_t*)((int)table | PDENT_GLOBAL | PDENT_PRESENT | PDENT_RW);
      virtual_dir[ DIR_OFFSET(addr) ] = table;
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
* @return The physical address, or NULL if the page isn't framed.
*/
void* kvm_vtop(void* vaddr)
{
   void* paddr;
   assert(vaddr > (void*)KVM_START);
   
   page_dirent_t* dir = (page_dirent_t*)global_pcb()->dir_v;
   page_tablent_t* table = dir[ DIR_OFFSET(vaddr) ];
   assert(TABLE_PRESENT(table));
   
   table = (page_tablent_t*)PAGE_OF(table);
   paddr = (void*)table[ TABLE_OFFSET(vaddr) ];
   
   assert(PAGE_PRESENT(paddr));
   
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
*
* @return 0 on success, a negative integer on failure. 
*/
int kvm_new_directory(pcb_t* pcb)
{
   int i;
   page_dirent_t* global_dir = global_pcb()->dir_v;
   
   if(kvm_request_frames(0, 2) < 0)
      return ENOVM;
   
   page_dirent_t* dir_v = kvm_new_page();
   page_dirent_t* virtual_dir_v = kvm_new_page();
   
   assert(dir_v);
   assert(virtual_dir_v);
   
   debug_print("kvm", "Global directory at %p", global_dir);
   
   for(i = 0; i < DIR_OFFSET(USER_MEM_START); i++)
      virtual_dir_v[i] = (page_dirent_t)PAGE_OF(global_dir[i]);
   
   memcpy(dir_v, global_dir, 
      DIR_OFFSET(USER_MEM_START) * sizeof(page_tablent_t*));
   
   /* Need to acquire the new table lock here, otherwise a new global 
    *  table could get allocated, and we would never find out about it, 
    *  since we aren't on the global list.*/
   mutex_lock(&new_table_lock);
   
   /* When we do this copy the directory itself gets mapped as well. */
   for(i = DIR_OFFSET(kvm_bottom); i < DIR_SIZE; i++)
      virtual_dir_v[i] = (page_dirent_t)PAGE_OF(global_dir[i]);
  
   memcpy(dir_v + DIR_OFFSET(kvm_bottom), 
      global_dir + DIR_OFFSET(kvm_bottom), 
      (DIR_SIZE - DIR_OFFSET(kvm_bottom)) * sizeof(page_tablent_t*));
   
   pcb->dir_v = dir_v;
   pcb->dir_p = kvm_vtop(dir_v);
   pcb->virtual_dir = virtual_dir_v;

   global_list_add(pcb);
   mutex_unlock(&new_table_lock);
   
   return ESUCCESS;
}

