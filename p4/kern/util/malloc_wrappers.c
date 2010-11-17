/** 
* @file malloc_wrappers.c
* @brief Wrappers for the non threadsafe malloc routines. 
* 
*  Since our kernel is meant to be air-tight, we should
*   only invoke smalloc, smemalign,  and sfree. 
*
* @author Justin Scheiner
* @author Tim Wilson
*/
#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <mutex.h>
#include <malloc_wrappers.h>
#include <simics.h>
#include <debug.h>
#include <string.h>
#include <common_kern.h>
#include <ecodes.h>
#include <eflags.h>

/* safe versions of malloc functions */

static mutex_t heap_lock;
static long allocated;
static long long nallocs;
static long long nfrees;
void* heap_sanity_start;

/**
 * @brief Initialize the dynamic memory management library for the kernel.
 */
void alloc_init()
{
   allocated = 0;
   nallocs = 0;
   nfrees = 0;
   heap_sanity_start = _smalloc(1);
   _sfree(heap_sanity_start, 1);

   mutex_init(&heap_lock);
}

/**
 * @brief Allocate dynamic memory
 *
 * @param size The number of bytes to allocate.
 *
 * @return The memory allocated, or NULL on failure.
 */
void *malloc(size_t size)
{
   mutex_lock(&heap_lock);
   void* ret = _malloc(size);
   mutex_unlock(&heap_lock);
   
   return ret;
}

/**
 * @brief Allocate aligned dynamic memory
 *
 * @param alignment The byte boundary to align to
 * @param size The number of bytes to allocate.
 *
 * @return The memory allocated, or NULL on failure.
 */
void *memalign(size_t alignment, size_t size)
{
   mutex_lock(&heap_lock);
   void* ret = _memalign(alignment, size);
   mutex_unlock(&heap_lock);
   
   return ret;
}

/**
 * @brief Allocate dynamic memory and zero it out
 *
 * @param nelt The number of elements to allocate.
 * @param eltsize The size of each element
 *
 * @return The memory allocated, or NULL on failure.
 */
void *calloc(size_t nelt, size_t eltsize)
{
   mutex_lock(&heap_lock);
   void* ret = _calloc(nelt, eltsize);
   mutex_unlock(&heap_lock);
   
   return ret;
}

/**
 * @brief Reallocate dynamic memory
 *
 * @param buf The buffer to reallocate
 * @param size The new size of the buffer
 *
 * @return The memory allocated, or NULL on failure.
 */
void *realloc(void *buf, size_t new_size)
{
   mutex_lock(&heap_lock);
   void* ret = _realloc(buf, new_size);
   mutex_unlock(&heap_lock);
   
   return ret;
}

/**
 * @brief Free dynamic memory
 *
 * @param buf The memory to free
 */
void free(void *buf)
{
   mutex_lock(&heap_lock);
   _free(buf);
   mutex_unlock(&heap_lock);
   
   return;
}

/**
 * @brief Allocate dynamic memory that we promise to remember the size of.
 *
 * @param size The number of bytes to allocate.
 *
 * @return The memory allocated, or NULL on failure.
 */
void *smalloc(size_t size)
{
   mutex_lock(&heap_lock);
   void* ret = _smalloc(size);

   if(ret)
   {
      nallocs++;
      allocated += size;
      debug_print("malloc", "Malloc of size %d, allocated = %d", 
         size, allocated);
   }
   mutex_unlock(&heap_lock);
   
   return ret;
}

/**
 * @brief Allocate dynamic memory that we promise to remember the size 
 * of and zero it out
 *
 * @param nelt The number of elements to allocate.
 * @param eltsize The size of each element
 *
 * @return The memory allocated, or NULL on failure.
 */
void *scalloc(size_t nmemb, size_t size)
{
   void *ret = smalloc(nmemb * size);
   if (ret != NULL) 
   {   
      debug_print("malloc", 
         "scalloc of size %d", nmemb * size);
      
      memset(ret, 0, nmemb * size);
   }
   return ret;
}

/**
 * @brief Reallocate dynamic memory that we promise to remember the size
 * of.
 *
 * @param buf The buffer to reallocate
 * @param current_size The current size of the buffer
 * @param new_size The new size of the buffer
 *
 * @return The memory allocated, or NULL on failure.
 */
void *srealloc(void* buf, size_t current_size, size_t new_size)
{
   void* new_buf = smalloc(new_size);
   
   if(!new_buf)
      return NULL;
   
   debug_print("malloc", 
      "srealloc of size %d, from %d", current_size, new_size); 
      
   memcpy(new_buf, buf, current_size);
   sfree(buf, current_size);
   return new_buf;
}

/**
 * @brief Allocate aligned dynamic memory that we promise to remember the
 * size of
 *
 * @param alignment The byte boundary to align to
 * @param size The number of bytes to allocate.
 *
 * @return The memory allocated, or NULL on failure.
 */
void *smemalign(size_t alignment, size_t size)
{
   mutex_lock(&heap_lock);
   void* ret = _smemalign(alignment, size);
   
   if(ret)
   {
      nallocs++;
      allocated += size;
      debug_print("malloc", 
         "Smemalign of size %d, alignment %d, allocated = %d", 
         alignment, size, allocated);
   }
   mutex_unlock(&heap_lock);
   
   return ret;
}

/**
 * @brief Free dynamic memory
 *
 * @param buf The memory to free
 * @param The size of the memory region to fill
 */
void sfree(void *buf, size_t size)
{
   memset(buf, 0, size);
   mutex_lock(&heap_lock);
  
   assert(heap_sanity_start <= buf && buf < (void*)USER_MEM_START);
   _sfree(buf, size);
   
   nfrees++;
   allocated -= size;
   debug_print("malloc", "Free of size %d, allocated = %d", size, allocated);
   assert(nallocs - nfrees > 0);
   mutex_unlock(&heap_lock);
}

