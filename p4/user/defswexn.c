/** 
* @file defswexn.c
*
* @brief Implements a default software exception handler, for
*  simple programs that shouldn't need to worry about software exceptions.
*
*  Notably implements the default autostack. 
*
* @author Justin Scheiner
* @author Tim Wilson
*/

#include <common_user.h>
#include <stdlib.h>
#include <assert.h>
#include <syscall.h>
#include <ureg.h>
#include <simics.h>

#define SWEXN_STACKSIZE 0x200

#define ESUCCESS 0
#define EFAIL -1

/** @brief The stack that the below code executes on.
 *    Note that a single exception stack is only appropriate for 
 *    single threaded programs.
 **/
unsigned char _defswexn_stack_buf[SWEXN_STACKSIZE] = {0};
unsigned char* _defswexn_stack = (_defswexn_stack_buf + SWEXN_STACKSIZE);

/** 
* @brief Implements the autostack feature for simple user processes.
* 
* @param ureg The register state 
*
* @return ESUCCESS if the problem was corrected, 
*         EFAIL otherwise
*/
int _handle_pagefault(ureg_t* ureg)
{
   lprintf(" Handling a pagefault in userspace!");
   void* addr = (void*)ureg->cr2;
   
   /* Check for a page fault in the autostack region. */
   if((void*)USER_STACK_START < addr && addr < (void*)USER_STACK_BASE)
   {
      if(new_pages((void*)PAGE_OF(addr), PAGE_SIZE) < 0)
         return EFAIL;
      else 
         return ESUCCESS;
   }
   return EFAIL;
}

/** 
* @brief A default software exception handler for ordinary 
*  single threaded programs that need an autostack, or generally
*  need to do something reasonable during an exception.
* 
* @param arg NULL, for this particular software exception.
* @param ureg The user state that caused the exception.
*/
void _defswexn(void* arg, ureg_t* ureg)
{
   int result;
   assert(arg == NULL);
   switch(ureg->cause)
   {
      case IDT_PF: 
         result = _handle_pagefault(ureg);
         break;

      //TODO Install default handlers for the rest of the exceptions. 
      
      default: 
         /* FIXME Obnoxiously MAGIC_BREAK if we haven't implemented a handler
          *  for this particular exception. */
         MAGIC_BREAK;
         result = EFAIL;
   }
   
   /* Only reregister a handler if we succeeded in fixing the problem. */
   if(result >= 0)
      swexn(_defswexn_stack, _defswexn, 0, ureg);
}



