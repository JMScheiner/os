
#ifndef CONTEXT_SWITCH_FK34JHG4
#define CONTEXT_SWITCH_FK34JHG4

/** @brief Switch from executing in the current thread to executing
 *         in another thread.
 *
 * @param old_esp Address to store the stack pointer of the current thread
 *
 * @param new_esp Stack address to jump execution to.
 */
void context_switch(void **old_esp, void *new_esp);

/** 
* @brief Creates a new thread context identical to the current one. 
*  After this function returns, it is safe to add the new thread to the
*     scheduler queue, and it will begin running with an updated TCB, 
*     and with the same stack context / register state.
* 
* @param old_bottom The bottom of the stack we are executing on. 
* @param new_bottom The bottom of the stack we plan on adding to the scheduler. 
* @param new_esp A pointer to the saved esp in the new thread. 
*/
void duplicate_thread_context(void* old_bottom, void* new_bottom, void** new_esp);

/** 
* @brief Creates a new thread context identical to the current one. 
*  ...with the exception that it's page directory will point to new_directory.
*
* @param old_bottom The bottom of the stack we are executing on. 
* @param new_bottom The bottom of the stack we plan on adding to the scheduler. 
* @param new_esp A pointer to the saved esp in the new thread. 
* @param new_directory The page directory of the new process.
*/
void duplicate_proc_context(  
   void* old_bottom, void *new_bottom, void** new_esp, void* new_directory
);

#endif

