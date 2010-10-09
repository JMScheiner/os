#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>

/* safe versions of malloc functions */
void *malloc(size_t size)
{
   void* ret = _malloc(size);
   return ret;
}

void *memalign(size_t alignment, size_t size)
{
   void* ret = _memalign(alignment, size);
   return ret;
}

void *calloc(size_t nelt, size_t eltsize)
{
   void* ret = _calloc(nelt, eltsize);
   return ret;
}

void *realloc(void *buf, size_t new_size)
{
   void* ret = _realloc(buf, new_size);
   return ret;
}

void free(void *buf)
{
   _free(buf);
   return;
}

void *smalloc(size_t size)
{
   void* ret = _smalloc(size);
   return ret;
}

void *smemalign(size_t alignment, size_t size)
{
   void* ret = _smemalign(alignment, size);
   return ret;
}

void *scalloc(size_t size)
{
   return NULL;
}

void sfree(void *buf, size_t size)
{
   _sfree(buf, size);
   return;
}

