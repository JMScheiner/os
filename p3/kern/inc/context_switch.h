
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

#endif

