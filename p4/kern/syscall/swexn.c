/** 
* @file swexn.c
*
* @brief Implements the software exception system call. 
*
* @author Tim Wilson
* @author Justin Scheiner
* @date 2010-11-17
*/

#include <reg.h>
#include <ecodes.h>
#include <simics.h>
#include <ureg.h>
#include <vstring.h>
#include <thread.h>
#include <common_kern.h>
#include <mm.h>
#include <macros.h>
#include <swexn.h>

/** 
* @brief Installs or uninstalls a software handler for the current
*  thread of execution, and if appropriate, installs a new set of
*  register values before returning to userspace. 
*
*  Invoked as 
*     int swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg);
*
* 1. If esp3 and/or eip are zero, de-register an exception handler if one 
*  is registered; regardless of whether a previous handler was de-registered, 
*  do not attempt to register a handler.
* 
* 2. If both esp3 and eip are non-zero, attempt to register a software 
*  exception handler. By analogy to esp0, esp3 points to an address one word
*  higher than the first address that the kernel should use to push values
*  onto the exception stack; eip points to the first instruction of the 
*  handler function. 

* 3. If newureg is non-zero the kernel is requested to adopt the specified
*  register values before the system call returns to user space. 
*
* 4. If a single invocation of swexn() attempts to both register a handler 
*  and specify register values, and either one cannot be carried out, 
*  neither one will be. 
*
* 5. If the invocation is invalid (e.g. the kernel is unable to obtain a 
*  complete set of registers by dereferencing a non-zero newureg pointer), 
*  an error code less than zero is returned. 
* 
* @param reg The register state on entry to the handler.
*/
void swexn_handler(volatile regstate_t reg)
{
   void* esp3;
   void* eip;
   void* arg;
   ureg_t* uregp;
   ureg_t ureg;
   boolean_t register_handler;
   
   char *arg_addr = (char *)SYSCALL_ARG(reg);
   tcb_t* tcb = get_tcb();
   
   /** Copy in arguments. **/
   if(v_copy_in_vptr(&esp3, arg_addr) < 0)
      RETURN(EARGS);
   
   if(v_copy_in_vptr(&eip, arg_addr + sizeof(void*)) < 0)
      RETURN(EARGS);
   
   if(v_copy_in_vptr(&arg, arg_addr + 2 * sizeof(void*)) < 0)
      RETURN(EARGS);
   
   if(v_copy_in_uregptr(&uregp, arg_addr + 3 * sizeof(void*)) < 0)
      RETURN(EARGS);
   
   /** Unregister, or reject bad values. */
   register_handler = (esp3 != NULL) && (eip != NULL);
   if(!register_handler)
   {
      tcb->handler.esp3 = NULL;
      tcb->handler.eip = NULL;
      tcb->handler.arg = NULL;
   }
   else if((esp3 < (void*)USER_MEM_START || esp3 >= (void*)USER_MEM_END) ||
    (eip < (void*)USER_MEM_START || eip >= (void*)USER_MEM_END))
   {
      /* Reject "clearly wrong" values of eip and esp3. 
       *  TODO Reject %eip's outside of the text region.
       **/
      RETURN(EFAIL);
   }
   
   /* Install the new register state if we can. 
    *  - Note that installing behavior is undefined if we are unregistering 
    *    a handler.  I chose to let it succeed.
    **/
   if(uregp != NULL)
   {
      if(v_memcpy((char*)&ureg, (char*)uregp, sizeof(ureg_t), TRUE) < 0)
         RETURN(EBUF);
      
      /* Only install values that it is safe for the user to modify. */
      reg.eip = ureg.eip;    
      reg.esp = ureg.esp;    
      reg.pusha.edi = ureg.edi;
      reg.pusha.esi = ureg.esi;
      reg.pusha.ebp = ureg.ebp;
      reg.pusha.ebx = ureg.ebx;
      reg.pusha.edx = ureg.edx;
      reg.pusha.ecx = ureg.ecx;
      reg.pusha.eax = ureg.eax;
   }

   /* If we've made it to this point, it's safe to to install the handler. */
   if(register_handler)
   {
      tcb->handler.esp3 = esp3;
      tcb->handler.eip = eip;
      tcb->handler.arg = arg;
   }

   RETURN(ESUCCESS);
}

/** 
* @brief Checks for a registered exception handler, 
*  and if it finds one, builds a context for the 
*  exception handler and executes it. 
* 
* This function never returns unless an error occurs.
*/
void swexn_try_invoke_handler(ureg_t* ureg)
{
   tcb_t *tcb = get_tcb();
   if (tcb->handler.eip == NULL) {
      /* No handler is registered. */
      return;
   }

   /* Deregister the current handler. */
   void *esp3 = tcb->handler.esp3;
   void *eip = tcb->handler.eip;
   void *arg = tcb->handler.arg;
   tcb->handler.esp3 = NULL;
   tcb->handler.eip = NULL;
   tcb->handler.arg = NULL;
   
   /* Copy the ureg state of the thread when the exception was invoked to
    * the exception stack. */
   char *stack_ptr = ALIGN_DOWN((char *)esp3 - sizeof(ureg_t), 
         sizeof(char *));
   if (v_memcpy(stack_ptr, (char *)ureg, sizeof(ureg_t), FALSE) != sizeof(ureg_t)) {
      /* We failed to write to the user exception stack. */
      return;
   }
   
   /* Copy a fake call frame to the exception stack to make it appear to
    * the user program as if the exception handler was called directly.
    * The return address (NULL) in this frame is invalid. Exception
    * handlers should never return. */
   void *frame[] = {NULL, stack_ptr, arg};
   stack_ptr -= sizeof(frame);
   if (v_memcpy(stack_ptr, (char *)frame, sizeof(frame), FALSE) != sizeof(frame)) {
      /* We failed to write to the user exception stack. */
      return;
   }

   /* Return to the user in their software exception handler. */
   swexn_return(eip, ureg->cs, ureg->eflags, stack_ptr, ureg->ss);
   assert(FALSE);
}




