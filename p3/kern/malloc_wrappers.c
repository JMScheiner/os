/** 
* @file malloc_wrappers.c
* @brief Wrappers for the non threadsafe malloc routines. 
* @author Justin Scheiner
* @author Tim Wilson
* @bug scalloc doesn't do anything. 
*/
#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <mutex.h>
#include <malloc_wrappers.h>
#include <simics.h>

/* safe versions of malloc functions */

static mutex_t heap_lock;

void alloc_init()
{
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
   return ret;
}

void *smemalign(size_t alignment, size_t size)
{
   mutex_lock(&heap_lock);
   void* ret = _smemalign(alignment, size);
   mutex_unlock(&heap_lock);
   return ret;
}

void *scalloc(size_t size)
{
   mutex_lock(&heap_lock);
   mutex_unlock(&heap_lock);
   return NULL;
}

void sfree(void *buf, size_t size)
{
   mutex_lock(&heap_lock);
   _sfree(buf, size);
   mutex_unlock(&heap_lock);
   return;
}

