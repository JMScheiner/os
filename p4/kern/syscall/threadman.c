/** @file threadman.c
 *
 * @brief Implementations of the thread management system call handlers.
 *
 * @author Tim Wilson
 * @author Justin Scheiner
 */
#include <threadman.h>
#include <thread.h>
#include <scheduler.h>
#include <simics.h>
#include <timer.h>
#include <ecodes.h>
#include <mutex.h>
#include <hashtable.h>
#include <vstring.h>
#include <debug.h>

/** 
* @brief Returns the TID of the current thread in %eax. 
* 
* @param reg The register state on entry and exit of the handler.
*/
void gettid_handler(volatile regstate_t reg)
{
   RETURN(get_tcb()->tid);
}

/** 
* @brief Defers execution of the invoking thread to a time determined
*  by the scheduler, in favor of the thread with ID in %esi.
*
*  If %esi is -1, the kernel may determine which thread to run next.
*
*  The only threads whose scheduling should be affected by yield() are 
*     the calling thread and the thread that is yield()ed to. 
*  
*  If the thread with ID tid does not exist, is awaiting an external event 
*     in a system call such as readline() or wait(), or has been suspended 
*     via a system call, then an integer error code less than zero is 
*     returned in %eax. 
*  
*  Zero is returned in %eax on success.
* 
* @param reg The register state on entry and exit of the handler.
*/
void yield_handler(volatile regstate_t reg)
{
   int tid = (int)SYSCALL_ARG(reg);
   if (tid == -1) {
      debug_print("yield", "%d yielding to anyone", get_tcb()->tid);
      quick_lock();
      scheduler_next();
      RETURN(ESUCCESS);
   }
   else {
      /* Find the thread in the tcb table. Hold onto the table lock so the
       * thread cannot disappear. */
      mutex_lock(&tcb_table()->lock);
      tcb_t *next = hashtable_get(tcb_table(), tid);
      debug_print("yield", "%d yielding to %d", get_tcb()->tid, tid);
      if (next == NULL) {
         mutex_unlock(&tcb_table()->lock);
         debug_print("yield", "%d failed to find desired yield", 
               get_tcb()->tid);
         RETURN(ENAME);
      }
      else if (scheduler_run(next, &tcb_table()->lock)) {
         /* scheduler_run released our lock */
         RETURN(ESUCCESS);
      }
      else {
         /* scheduler_run released our lock */
         debug_print("yield", "%d desired yield is descheduled or blocked", 
               get_tcb()->tid);
         RETURN(ESTATE);
      }
   }
}

/** 
* @brief Atomically checks the integer pointed to by reject and acts on it. 
*
*  If the integer is non-zero, the call returns immediately with return 
*  value zero. 
*
*  If the integer pointed to by reject is zero, then the calling thread 
*  will not be run by the scheduler until a make runnable() call is made 
*  specifying the deschedule()'d thread, at which point deschedule() will 
*  return zero
*
*  An integer error code less than zero is returned in %eax if reject is 
*  not a valid pointer. 
*
*  This system call is atomic with respect to make runnable(): the process 
*  of examining reject and suspending the thread will not be interleaved 
*  with any execution of make runnable() specifying the thread calling 
*  deschedule().
*
* @param reg The register state on entry and exit of the handler.
*/
void deschedule_handler(volatile regstate_t reg)
{
   char *arg_addr = (char *)SYSCALL_ARG(reg);
   int reject;
   tcb_t *tcb = get_tcb();
   mutex_lock(&tcb->deschedule_lock);
   
   if(v_copy_in_int(&reject, arg_addr) < 0)
   {
      debug_print("deschedule", "Failed to copy reject arg");
      mutex_unlock(&tcb->deschedule_lock);
      RETURN(EARGS);
   }
   if (reject == 0) {
      debug_print("deschedule", "Descheduling %d now", get_tcb()->tid);
      /* deschedule will release our lock */
      scheduler_deschedule(&tcb->deschedule_lock);
      debug_print("deschedule", "Rescheduling %d now", get_tcb()->tid);
   }
   else {
      debug_print("deschedule", "Reject nonzero");
      mutex_unlock(&tcb->deschedule_lock);
   }
   RETURN(ESUCCESS);
}

/** 
* @brief Makes the deschedule()d thread with ID tid runnable by the 
* scheduler.   
*
*  On success, zero is returned in %eax. 
*
*  An integer error code less than zero will be returned in %eax unless 
*  tid is the ID of a thread which exists but is currently non-runnable 
*  due to a call to deschedule().
*
* @param reg The register state on entry and exit of the handler.
*/
void make_runnable_handler(volatile regstate_t reg)
{
   int tid = (int)SYSCALL_ARG(reg);
   int ret = 0;
   mutex_lock(&tcb_table()->lock);
   tcb_t *tcb = hashtable_get(tcb_table(), tid);
   debug_print("make_runnable", "Acquired deschedule_lock");
   if (tcb == NULL) {
      debug_print("make_runnable", "%d failed, target does not exist", 
            get_tcb()->tid);
      ret = ENAME;
   }
   else {
      mutex_lock(&tcb->deschedule_lock);
      if (!scheduler_reschedule(tcb)) {
         debug_print("make_runnable", "%d failed, %d is already runnable", 
               tcb->tid, get_tcb()->tid);
         ret = ESTATE;
      }
      mutex_unlock(&tcb->deschedule_lock);
   }
   mutex_unlock(&tcb_table()->lock);
   RETURN(ret);
}

/** 
* @brief Returns the number of timer ticks which have occurred since 
* system boot in %eax.
* 
* @param reg The register state on entry and exit of the handler.
*/
void get_ticks_handler(volatile regstate_t reg)
{
   RETURN(time());
}

/** 
* @brief Deschedules the calling thread until at least ticks timer 
* interrupts have occurred after the call. 
*
*  Returns immediately if ticks is zero. 
*
*  Returns an integer error code less than zero in %eax if ticks is 
*  negative. Returns zero in %eax otherwise.
*
*  In a _very_ exceptional case, we can fail to reallocate the sleep 
*   heap, and we are required to say no to sleep requests.
* 
* @param reg The register state on entry and exit of the handler.
*/
void sleep_handler(volatile regstate_t reg)
{
   int ticks = (int)SYSCALL_ARG(reg);
   
   if(ticks < 0) RETURN(EARGS); 
   if(ticks == 0) RETURN(ESUCCESS);

   RETURN(scheduler_sleep(ticks));
}


