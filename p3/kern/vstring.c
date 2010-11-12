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
#include <ecodes.h>
#include <mm.h>

/**
 * @brief Check that a user has permissions to read from a given address.
 *
 * @param addr The address to check.
 *
 * @return True iff the user can safely read from the address.
 */
static boolean_t validate_user_read(void* addr)
{
   int flags = mm_getflags(addr);
   return (flags == (flags | PTENT_USER | PTENT_PRESENT));
}

/**
 * @brief Check that a user has permissions to write to a given address.
 *
 * @param addr The address to check.
 *
 * @return True iff the user can safely write to the address.
 */
static boolean_t validate_user_write(void* addr)
{
   int flags = mm_getflags(addr);
   return (flags == (flags | PTENT_USER | PTENT_PRESENT | PTENT_RW));
}

/**
 * @brief Check that the kernel has permissions to read from a given address.
 *
 * @param addr The address to check.
 *
 * @return True iff the kernel can safely read from the address.
 */
static boolean_t validate_kernel_read(void* addr)
{
   int flags = mm_getflags(addr);
   return (flags == (flags | PTENT_PRESENT));
}

/**
 * @brief Check that the kernel has permissions to write to a given address.
 *
 * @param addr The address to check.
 *
 * @return True iff the kernel can safely write to the address.
 */
static boolean_t validate_kernel_write(void* addr)
{
   int flags = mm_getflags(addr);
   return (flags == (flags | PTENT_PRESENT | PTENT_RW));
}

/**
 * @brief Perform a checked copy of memory between user space and kernel
 * space.
 *
 * @param dst The location to copy into.
 * @param src The location to copy from.
 * @param max_len The maximum number of characters to copy.
 * @param True if we are copying from user memory to kernel memory, false
 * otherwise.
 * @param True if we are copying a nul terminated string, false otherwise.
 *
 * @return The number of bytes copied, or a negative error code.
 */
int v_cpy(char *dst, char *src, int max_len, 
		boolean_t user_source, boolean_t copying_string) {
   int n;
   boolean_t (*validate_write)(void*);
   boolean_t (*validate_read)(void*);
   
   if(user_source) {
      validate_read = validate_user_read;
      validate_write = validate_kernel_write;
   }
   else {
      validate_read = validate_kernel_read;
      validate_write = validate_user_write;
   }

   mutex_t* lock = new_pages_lock();
   mutex_lock(lock);
   
   if(!validate_read(src) || !validate_write(dst)) {
      mutex_unlock(lock);
      return EBUF;
   }
   
   for(n = 0; n < max_len; n++, src++, dst++) {
      /* Ensure that we can write and read to the next addresses. */
      if(!SAME_PAGE(src, src - 1) && (!validate_read(src)))
         break;
         
      if(!SAME_PAGE(dst, dst - 1) && (!validate_write(dst))) 
         break;
		
		if (copying_string) {
	      if((*dst = *src) == '\0') {
      	   mutex_unlock(lock);
				return n + 1;
			}
		}
		else {
			*dst = *src;
		}
   }
   
   mutex_unlock(lock);
   
	if (!copying_string)
		return n;
	else if (n == max_len)
		return ELEN;
   else
      return EBUF;
}

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
 * @param True if we are copying from user memory to kernel memory, false
 * otherwise.
 *
 * @return The number of characters copied. Or a negative number on failure. 
 */
int v_strcpy(char *dst, char *src, int max_len, boolean_t user_source)
{
	return v_cpy(dst, src, max_len, user_source, TRUE);
}

/**
 * @brief Perform a validated memcpy
 
 * Verify that each byte is mapped, and terminate early when reaching 
 *  an unmapped byte or max_len. 
 *
 * @param dst The location to copy into (kernel memory)
 * @param src The location to copy from (user memory)
 * @param max_len The number of bytes to copy.
 * @param True if we are copying from user memory to kernel memory, false
 * otherwise.
 *
 * @return The number of bytes copied (may be less then len).
 */
int v_memcpy(char *dst, char *src, int len, boolean_t user_source) 
{
	return v_cpy(dst, src, len, user_source, FALSE);
}

