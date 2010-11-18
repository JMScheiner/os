/** 
* @file vstring.h
* @brief Safe copying from user to kernel memory and back again. 
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-11-12
*/

#ifndef VALIDATION_H_FJWK2143
#define VALIDATION_H_FJWK2143

#include <mm.h>
#include <ecodes.h>
#include <ureg.h>

int v_strcpy(char *dest, char *src, int max_len, boolean_t user_source);
int v_memcpy(char *dest, char *src, int max_len, boolean_t user_source);

/* Various utility functions for more specific argument copying: */

/** 
* @brief Copies in a char* from user memory. 
* 
* @param ptr A local copy of the pointer. 
* @param arg_addr The address of the user argument. 
* 
* @return ESUCCESS on success, EFAIL on failure. 
*/
static inline int v_copy_in_ptr(char** ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(char*), TRUE);
   if(ret < sizeof(char*))
      return EFAIL;
   else return ESUCCESS;
}

/** 
* @brief Copies in a void* from user memory. 
* 
* @param ptr A local copy of the pointer. 
* @param arg_addr The address of the user argument. 
* 
* @return ESUCCESS on success, EFAIL on failure. 
*/
static inline int v_copy_in_vptr(void** ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(void*), TRUE);
   if(ret < sizeof(void*))
      return EFAIL;
   else return ESUCCESS;
}

/** 
* @brief Copies in a void* from user memory. 
* 
* @param ptr A local copy of the pointer. 
* @param arg_addr The address of the user argument. 
* 
* @return ESUCCESS on success, EFAIL on failure. 
*/
static inline int v_copy_in_uregptr(ureg_t** ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(void*), TRUE);
   if(ret < sizeof(ureg_t*))
      return EFAIL;
   else return ESUCCESS;
}

/** 
* @brief Copies in a char** from user memory. 
* 
* @param ptr A local copy of the char** 
* @param arg_addr The address of the user argument. 
* 
* @return ESUCCESS on success, EFAIL on failure. 
*/
static inline int v_copy_in_dptr(char*** ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(char**), TRUE);
   if(ret < sizeof(char**))
      return EFAIL;
   else return ESUCCESS;
}

/** 
* @brief Copies in an int from user memory. 
* 
* @param ptr A local copy of the int
* @param arg_addr The address of the user argument. 
* 
* @return ESUCCESS on success, EFAIL on failure. 
*/
static inline int v_copy_in_int(int* ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(int), TRUE);
   if(ret < sizeof(int))
      return EFAIL; 
   else return ESUCCESS;
}

/** 
* @brief Copies in an int* from user memory. 
* 
* @param ptr A local copy of the int*
* @param arg_addr The address of the user argument. 
* 
* @return ESUCCESS on success, EFAIL on failure. 
*/
static inline int v_copy_in_intptr(int** ptr, char* arg_addr)
{
   int ret = v_memcpy((char*)ptr, arg_addr, sizeof(int*), TRUE);
   if(ret < sizeof(int*))
      return EFAIL; 
   else return ESUCCESS;
}

/** 
* @brief Copies an int back out to user memory. 
* 
* @param dst The pointer into user memory. 
* @param src The value to copy out. 
* 
* @return ESUCCESS on success, EFAIL on failure. 
*/
static inline int v_copy_out_int(int* dst, int src)
{
   int ret = v_memcpy((char*)dst, (char*)&src, sizeof(int), FALSE);
   if(ret < sizeof(int))
      return EFAIL; 
   else return ESUCCESS;
}

#endif
