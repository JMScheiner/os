
#ifndef VALIDATION_H_FJWK2143
#define VALIDATION_H_FJWK2143

#include <mm.h>

/**
 * @brief Perform a checked loop over a region of memory
 *
 * @param addr The variable that will loop over the memory region
 * @param inc The number of bytes to increment the address by on each iteration
 *            of the loop.
 * @param cntr A variable counting the number of iterations performed by the
 *             loop. Need not be initialized.
 * @param max The maximum number of iterations to perform.
 */
#define SAFE_LOOP(addr, cntr, max) \
	for ((cntr) = 0 ; \
			((cntr) == (max)) ? 0 : \
         ((cntr) == 0 || !SAME_PAGE(addr, (addr) - 1)) ? \
            mm_validate_read(addr, 1) : 1 ; \
         (cntr)++, (addr)++)

#define NOT_NULL_TERMINATED -1
#define INVALID_MEMORY -2

int v_strcpy(char *dest, char *src, int max_len, boolean_t user_source);
int v_memcpy(char *dest, char *src, int max_len, boolean_t user_source);

/* Various utility functions for more specific argument copying: */

static inline int v_copy_in_ptr(char** ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(char*), TRUE);
   if(ret < sizeof(char*))
      return -1;
   else return 0;
}

static inline int v_copy_in_dptr(char*** ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(char**), TRUE);
   if(ret < sizeof(char**))
      return -1;
   else return 0;
}

static inline int v_copy_in_int(int* ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(int), TRUE);
   if(ret < sizeof(int))
      return -1; 
   else return 0;
}

static inline int v_copy_in_intptr(int** ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(int*), TRUE);
   if(ret < sizeof(int*))
      return -1; 
   else return 0;
}

static inline int v_copy_out_int(int* dst, int src)
{
   int ret = v_memcpy((char*)dst, (char*)&src, sizeof(int), FALSE);
   if(ret < sizeof(int))
      return -1; 
   else return 0;
}



#endif
