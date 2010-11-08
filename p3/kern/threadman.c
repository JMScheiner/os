
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

mutex_t deschedule_lock;

void threadman_init() {
	mutex_init(&deschedule_lock);
}

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
	quick_lock();
	if (tid == -1) {
		debug_print("yield", "%d yielding to anyone", get_tcb()->tid);
		scheduler_next();
		RETURN(E_SUCCESS);
	}
	else {
		// No need to lock, interrupts are disabled
		tcb_t *next = hashtable_get(&tcb_table, tid);
		if (next == NULL) {
			debug_print("yield", "%d failed to find desired yield", get_tcb()->tid);
			quick_unlock();
			RETURN(YIELD_NONEXISTENT);
		}
		else if (next->descheduled || next->blocked) {
			debug_print("yield", "%d desired yield is descheduled or blocked", get_tcb()->tid);
			quick_unlock();
			RETURN(YIELD_BLOCKED);
		}
		debug_print("yield", "%d yielding to %d", get_tcb()->tid, tid);
		scheduler_run(next);
		RETURN(E_SUCCESS);
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
	int *reject = (int *)SYSCALL_ARG(reg);

   //FIXME We don't atomically dereference reject! We don't validate it!
   // What we need to do is validate, hold remove pages lock, and atomic exchange.
   // ...or just validate and read in scheduler_deschedule.

	mutex_lock(&deschedule_lock);
	if (*reject == 0) {
		debug_print("deschedule", "Descheduling %d now", get_tcb()->tid);
		scheduler_deschedule(&deschedule_lock);
		debug_print("deschedule", "Rescheduling %d now", get_tcb()->tid);
	}
	else {
		debug_print("deschedule", "Reject nonzero");
		mutex_unlock(&deschedule_lock);
	}
	RETURN(E_SUCCESS);
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
	mutex_lock(&tcb_table.lock);
	tcb_t *tcb = hashtable_get(&tcb_table, tid);
	mutex_lock(&deschedule_lock);
	debug_print("make_runnable", "Acquired deschedule_lock");
	if (tcb == NULL) {
		debug_print("make_runnable", "%d failed, does not exist", get_tcb()->tid);
		ret = MAKE_RUNNABLE_NONEXISTENT;
	}
	else if (!scheduler_reschedule(tcb)) {
		debug_print("make_runnable", "%d failed, already runnable", get_tcb()->tid);
		ret = MAKE_RUNNABLE_SCHEDULED;
	}
	mutex_unlock(&deschedule_lock);
	mutex_unlock(&tcb_table.lock);
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
* @param reg The register state on entry and exit of the handler.
*/
void sleep_handler(volatile regstate_t reg)
{
   int ticks = (int)SYSCALL_ARG(reg);
   
   if(ticks < 0) RETURN(SYSCALL_INVALID_ARGS); 
   if(ticks == 0) RETURN(E_SUCCESS);

   scheduler_sleep(ticks);
   RETURN(E_SUCCESS);
}


