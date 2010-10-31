
#include <threadman.h>
#include <thread.h>
#include <scheduler.h>
#include <simics.h>
#include <timer.h>
#include <syscall_codes.h>

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
*     via a system call, then an integer error code less than zero is returned 
*     in %eax. 
*  
*  Zero is returned in %eax on success.
* 
* @param reg The register state on entry and exit of the handler.
*/
void yield_handler(volatile regstate_t reg)
{
   //int tid = (int)SYSCALL_ARG(reg);
	lprintf("Ignoring yield");
	MAGIC_BREAK;
   //TODO
}

/** 
* @brief Atomically checks the integer pointed to by reject and acts on it. 
*
*  If the integer is non-zero, the call returns immediately with return value zero. 
*
*  If the integer pointed to by reject is zero, then the calling thread will not be
*     run by the scheduler until a make runnable() call is made specifying the 
*     deschedule()'d thread, at which point deschedule() will return zero
*
*  An integer error code less than zero is returned in %eax if reject is not a valid pointer. 
*
*  This system call is atomic with respect to make runnable(): the process of examining 
*     reject and suspending the thread will not be interleaved with any execution of 
*     make runnable() specifying the thread calling deschedule().
*
* @param reg The register state on entry and exit of the handler.
*/
void deschedule_handler(volatile regstate_t reg)
{
	lprintf("Ignoring deschedule");
	MAGIC_BREAK;
   //TODO
}

/** 
* @brief Makes the deschedule()d thread with ID tid runnable by the scheduler.   
*
*  On success, zero is returned in %eax. 
*
*  An integer error code less than zero will be returned in %eax unless tid is 
*     the ID of a thread which exists but is currently non-runnable due to a 
*     call to deschedule().
*
* @param reg The register state on entry and exit of the handler.
*/
void make_runnable_handler(volatile regstate_t reg)
{
	lprintf("Ignoring make_runnable");
	MAGIC_BREAK;
   //TODO
}

/** 
* @brief Returns the number of timer ticks which have occurred since system boot in %eax.
* 
* @param reg The register state on entry and exit of the handler.
*/
void get_ticks_handler(volatile regstate_t reg)
{
   RETURN(time());
}

/** 
* @brief Deschedules the calling thread until at least ticks timer interrupts have occurred 
*     after the call. 
*
*  Returns immediately if ticks is zero. 
*
*  Returns an integer error code less than zero in %eax if ticks is negative. 
*  Returns zero in %eax otherwise.
* 
* @param reg The register state on entry and exit of the handler.
*/
void sleep_handler(volatile regstate_t reg)
{
   int ticks = (int)SYSCALL_ARG(reg);
   
   if(ticks < 0) RETURN(SYSCALL_INVALID_ARGS); 
   if(ticks == 0) RETURN(SYSCALL_SUCCESS);

   scheduler_sleep(ticks);
   RETURN(SYSCALL_SUCCESS);
}





