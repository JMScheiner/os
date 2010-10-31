/** 
* @file vstring.c
*
* @brief Provides the functionality of string.h 
*  while validating the source and dstination, one of which 
*  may belong to the user. 
*
*  In terms of race conditions - we are really only worried about the 
*     interaction between these functions and remove_pages, since that
*     is the only mechanism for another thread in the same address 
*     space to mess things up. There is a single lock that protects us
*     from the effects of remove_pages. 
*
* @author Tim Wilson
* @author Justin Scheiner
*/

#include <vstring.h>
#include <memman.h>
#include <mutex.h>
#include <simics.h>

#define CHECK_SRC(src) \
   if((!SAME_PAGE(src, src - 1)) && \
      !mm_validate_read(src, 1)) break;

#define CHECK_DEST(dst) \
   if((!SAME_PAGE(dst, dst - 1)) && \
      !mm_validate_read(dst, 1)){  break; }

/**
 * @brief Perform a validated strcpy.
 * 
 * Copy a null terminated string when there is a possibility that src
 *    and dst are unmapped memory (i.e. one of the buffers belongs
 *    to the user.) 
 * 
 * Verify that each byte is mapped, and terminate early when reaching 
 *  an unmapped byte or max_len. 
 *
 * @param dst The location to copy into.
 * @param src The location to copy from.
 * @param max_len The maximum number of characters to copy.
 *
 * @return The number of characters copied. Or a negative number on failure. 
 */
int v_strcpy(char *dst, char *src, int max_len) {
	int i;
   
   mutex_t* lock = remove_pages_lock();
   mutex_lock(lock);
   
   if(!mm_validate_read(src, 1)){
      MAGIC_BREAK;
      mutex_unlock(lock);
      return INVALID_MEMORY;
   }
   if(!mm_validate_write(dst, 1)){
      MAGIC_BREAK;
      mutex_unlock(lock);
      return INVALID_MEMORY;
   }
      
   for(i = 0; i < max_len; i++, src++, dst++)
   {
      /* Ensure that we can write and read to the next addresses. */
      CHECK_SRC(src);
      CHECK_DEST(dst);
      
      if((*dst = *src) == '\0') 
      {
         mutex_unlock(remove_pages_lock());
			return i + 1;
		}
   }
   
   mutex_unlock(lock);
   
   if (i == max_len) {
		return NOT_NULL_TERMINATED;
	}
   return INVALID_MEMORY;
}

/**
 * @brief Perform a validated memcpy
 *
 * @param dst The location to copy into (kernel memory)
 * @param src The location to copy from (user memory)
 * @param max_len The number of bytes to copy.
 *
 * @return The number of bytes copied (may be less then len).
 */
int v_memcpy(char *dst, char *src, int len) 
{
	int i;
   mutex_t* lock = remove_pages_lock();
   mutex_lock(lock);
   
   if(!mm_validate_read(src, 1)){
      MAGIC_BREAK;
      mutex_unlock(lock);
      return INVALID_MEMORY;
   }
   if(!mm_validate_write(dst, 1)){
      MAGIC_BREAK;
      mutex_unlock(lock);
      return INVALID_MEMORY;
   }
      
   for(i = 0; i < len; i++, src++, dst++)
   {
      /* Ensure that we can write and read to the next addresses. */
      CHECK_SRC(src);
      CHECK_DEST(dst);
      *dst = *src; 
   }
   
   mutex_unlock(lock);
	return i;
}

