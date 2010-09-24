/*
 * these functions should be thread safe.
 * It is up to you to rewrite them
 * to make them thread safe.
 *
 * Justin - All I did here was wrap the nonthreadsafe functions
 * 	in a "heap lock." Further thought may be required.
 *
 */

#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include <mutex.h>

mutex_t heap_lock;

void *malloc(size_t __size)
{
	void* ret;
	
	mutex_lock(&heap_lock);
	
	ret = _malloc(__size);	
	
	mutex_unlock(&heap_lock);
	
	return ret;
}

void *calloc(size_t __nelt, size_t __eltsize)
{
	void* ret;
	
	mutex_lock(&heap_lock);
	
	ret = _calloc(__nelt, __eltsize);	
	
	mutex_unlock(&heap_lock);

	return ret;
}

void *realloc(void *__buf, size_t __new_size)
{
	void* ret;
	
	mutex_lock(&heap_lock);
	
	ret = _realloc(__buf, __new_size);	
	
	mutex_unlock(&heap_lock);
	
	return ret;
}

void free(void *__buf)
{
	mutex_lock(&heap_lock);
	
	free(__buf);	
	
	mutex_unlock(&heap_lock);
}
