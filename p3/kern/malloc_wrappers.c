/** 
* @file malloc_wrappers.c
* @brief Wrappers for the non threadsafe malloc routines. 
* 
*  Since our kernel is meant to be air-tight, we should
*   only invoke smalloc, smemalign,  and sfree. 
*
*   TODO Outstanding ssues with memory management:
*     - Need to cover memalign calls. 
*     - Root out where we malloc, but don't initialize
*        (I've found a few)
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

/* safe versions of malloc functions */

static mutex_t heap_lock;
static long allocated;
static long long nallocs;
static long long nfrees;

void alloc_init()
{
   allocated = 0;
   nallocs = 0;
   nfrees = 0;
	mutex_init(&heap_lock);
}

void *malloc(size_t size)
{
   mutex_lock(&heap_lock);
   void* ret = _malloc(size);
   mutex_unlock(&heap_lock);
   
   return ret;
}

void *memalign(size_t alignment, size_t size)
{
   mutex_lock(&heap_lock);
   void* ret = _memalign(alignment, size);
   mutex_unlock(&heap_lock);
   
   return ret;
}

void *calloc(size_t nelt, size_t eltsize)
{
   mutex_lock(&heap_lock);
   void* ret = _calloc(nelt, eltsize);
   mutex_unlock(&heap_lock);
   
   return ret;
}

void *realloc(void *buf, size_t new_size)
{
   mutex_lock(&heap_lock);
   void* ret = _realloc(buf, new_size);
   mutex_unlock(&heap_lock);
   
   return ret;
}

void free(void *buf)
{
   mutex_lock(&heap_lock);
   _free(buf);
   mutex_unlock(&heap_lock);
   
   return;
}

void *smalloc(size_t size)
{
   mutex_lock(&heap_lock);
   void* ret = _smalloc(size);
   mutex_unlock(&heap_lock);


   if(ret)
   {
      nallocs++;
      allocated += size;
      debug_print("malloc", "Malloc of size %d, allocated = %d", 
         size, allocated);
   }
   
   return ret;
}

void *scalloc(size_t nmemb, size_t size)
{
	void *ret = smalloc(nmemb * size);
	memset(ret, 0, nmemb * size);
	return ret;
}

void *srealloc(void* buf, size_t current_size, size_t new_size)
{
   if(new_size > current_size)
   {
      void* new_buf = smalloc(new_size);
      
      if(!new_buf)
         return NULL;
      
      memcpy(new_buf, buf, current_size);
      sfree(buf, current_size);
      return new_buf;
   }
   else return buf;
}

void *smemalign(size_t alignment, size_t size)
{
   mutex_lock(&heap_lock);
   void* ret = _smemalign(alignment, size);
   mutex_unlock(&heap_lock);
   
   if(ret)
   {
      nallocs++;
      allocated += size;
      debug_print("malloc", 
         "Smemalign of size %d, alignment %d, allocated = %d", 
         alignment, size, allocated);
   }
   
   return ret;
}

void sfree(void *buf, size_t size)
{
   mutex_lock(&heap_lock);
   _sfree(buf, size);
   mutex_unlock(&heap_lock);
   
   nfrees++;
   allocated -= size;
   debug_print("malloc", "Free of size %d, allocated = %d", size, allocated);
   assert(nallocs - nfrees > 0);
}

