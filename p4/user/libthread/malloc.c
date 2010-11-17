/** @file malloc.c
 *
 * @brief Thread safe wrappers for memory management.
 *
 * @author Timothy Wilson (tjwilson)
 * @author Justin Scheiner (jscheine)
 */

#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include <stdio.h>
#include <mutex.h>
#include <simics.h>
#include <atomic.h>

/** @brief A lock for memory management functions. */
mutex_t heap_lock;

/** @brief This is 0 if the heap_lock has not yet been initialized. */
int lock_initialized = 0;

/** @brief Initialize the heap lock if it has not been initialized already.
 *
 * Enforces that mutex_init is called only once for the heap lock.
 */
static void try_initialize() {
   if(!lock_initialized && atomic_add(&lock_initialized, 1) == 0)
   {
      mutex_init(&heap_lock);
   }
}

/** @brief Thread safe wrapper for malloc.
 *
 * @param __size The number of bytes to allocate.
 *
 * @return a pointer to the memory allocated. NULL if allocation failed.
 */
void *malloc(size_t __size)
{
   void* ret;
   
   try_initialize();
   
   mutex_lock(&heap_lock);
   ret = _malloc(__size);  
   mutex_unlock(&heap_lock);
   return ret;
}

/** @brief Thread safe wrapper for calloc.
 *
 * @param __nelt The number of elements to allocate space for.
 * @param __eltsize The size of each element.
 *
 * @return a pointer to the memory allocated. NULL if allocation failed.
 */
void *calloc(size_t __nelt, size_t __eltsize)
{
   void* ret;
   
   try_initialize();

   mutex_lock(&heap_lock);
   ret = _calloc(__nelt, __eltsize);   
   mutex_unlock(&heap_lock);
   return ret;
}

/** @brief Thread safe wrapper for realloc.
 *
 * @param __buf The memory region to reallocate.
 * @param __new_size The new size of the memory region.
 *
 * @return a pointer to the memory allocated. NULL if allocation failed.
 */
void *realloc(void *__buf, size_t __new_size)
{
   void* ret;

   try_initialize();

   mutex_lock(&heap_lock);
   ret = _realloc(__buf, __new_size);  
   mutex_unlock(&heap_lock);
   return ret;
}

/** @brief Thread safe wrapper for free.
 *
 * @param __buf The memory region to free.
 */
void free(void *__buf)
{
   try_initialize();
   
   mutex_lock(&heap_lock);
   _free(__buf);  
   mutex_unlock(&heap_lock);
}

