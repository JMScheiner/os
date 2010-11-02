/** 
* @file global_thread.h
*
* @brief These structs represent the "global"
*    PCB and TCB that stand in when there isn't
*    an appropriate thread in places where we need
*    one. Examples   
*     - if the only thread sleeps
*     - if we are executing with the global directory and no
*        TCB / PCB of our own. 
*
* @author Justin Scheiner
* @date 2010-10-26
*/

#ifndef GLOBAL_THREAD_FZQ3AUU8
#define GLOBAL_THREAD_FZQ3AUU8

#include <kernel_types.h>

void global_thread_init(void);
inline pcb_t* global_pcb(void);
inline tcb_t* global_tcb(void);
inline mutex_t* global_list_lock(void);

#endif /* end of include guard: GLOBAL_THREAD_FZQ3AUU8 */

