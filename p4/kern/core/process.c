/**
 * @file process.c
 * @brief Functions related to process intialization and destruction.
 *
 * @author Tim Wilson
 * @author Justin Scheiner
 */

#include <process.h>
#include <mm.h>
#include <assert.h>
#include <hashtable.h>
#include <string.h>
#include <atomic.h>
#include <page.h>
#include <region.h>
#include <thread.h>
#include <mutex.h>
#include <global_thread.h>
#include <cond.h>
#include <kvm.h>
#include <ecodes.h>
#include <simics.h>
#include <malloc_wrappers.h>
#include <macros.h>

/**
 * @brief Next pid to assign to a process.
 */
static int next_pid = 1;

/** @brief pcb of the init process. */
pcb_t *init_process;

/** 
* @brief Frees everything except for the status - 
*  which is required to remain around for calls to wait.
* 
* @param pcb The pcb for the process. 
* @param vanishing True if we are running in the PCB's address space. 
*     (Which only occurs when we are vanishing) 
*/
void free_process_resources(pcb_t* pcb, boolean_t vanishing)
{
   assert(pcb);
   assert(pcb->thread_count == 0);
   assert(pcb->sanity_constant == PCB_SANITY_CONSTANT);
   
   free_region_list(pcb);
   mm_free_address_space(pcb);
   
   mutex_destroy(&pcb->directory_lock);
   mutex_destroy(&pcb->region_lock);
   mutex_destroy(&pcb->status_lock);
   mutex_destroy(&pcb->waiter_lock);
   mutex_destroy(&pcb->check_waiter_lock);
   mutex_destroy(&pcb->child_lock);
   mutex_destroy(&pcb->swexn_lock);
   cond_destroy(&pcb->wait_signal);
   cond_destroy(&pcb->vanish_signal);
   cond_destroy(&pcb->swexn_signal);
   sfree(pcb, sizeof(pcb_t));
}

/**
 * @brief Get the pcb of the currently running process.
 *
 * @return The pcb.
 */
pcb_t* get_pcb()
{
   pcb_t* pcb;
   tcb_t* tcb;
   
   tcb = get_tcb();
   assert(tcb);
   pcb = tcb->pcb;
   assert(pcb->sanity_constant == PCB_SANITY_CONSTANT);
   return pcb;
}

/**
 * @brief Initialize a process
 *
 * @param first_process True iff this is the init process that is being
 * hand loaded.
 *
 * @return The pcb of the new process on success, NULL on failure.
 */
pcb_t* initialize_process(boolean_t first_process) 
{
   pcb_t* pcb;
   
   if((pcb = (pcb_t*) scalloc(1, sizeof(pcb_t))) < 0) 
      goto fail_pcb;

   LIST_INIT_EMPTY(pcb->children);
   LIST_INIT_NODE(pcb, global_node);
   LIST_INIT_NODE(pcb, child_node);
   
   if(kvm_new_directory(pcb) < 0) 
      goto fail_new_directory;
   
   pcb->pid = atomic_add(&next_pid, 1);
   if (first_process) {
      pcb->parent = global_pcb();
      init_process = pcb;
   }
   else {
      pcb->parent = get_pcb();
   }
   pcb->thread_count = 0;
   pcb->regions = NULL;
   
   if((pcb->status = (status_t *)scalloc(1, sizeof(status_t))) < 0) 
      goto fail_status;
   
   pcb->status->status = 0;
   pcb->status->next = NULL;
   
   pcb->unclaimed_children = 0;
   pcb->vanishing_children = 0;
   pcb->vanishing = FALSE;
   pcb->zombie_statuses = NULL;
   pcb->sanity_constant = PCB_SANITY_CONSTANT;
   
   mutex_init(&pcb->directory_lock);
   mutex_init(&pcb->region_lock);
   mutex_init(&pcb->status_lock);
   mutex_init(&pcb->waiter_lock);
   mutex_init(&pcb->check_waiter_lock);
   mutex_init(&pcb->child_lock);
   mutex_init(&pcb->swexn_lock);

   cond_init(&pcb->wait_signal);
   cond_init(&pcb->vanish_signal);
   cond_init(&pcb->swexn_signal);
   
   return pcb;

fail_status: 
   mm_free_address_space(pcb);
fail_new_directory:
   sfree(pcb, sizeof(pcb_t));
fail_pcb: 
   return NULL;
}

/**
 * @brief Get the pid of the current running process.
 *
 * @return The current process pid.
 */
int get_pid() {
   tcb_t *tcb = get_tcb();
   if (tcb == NULL) {
      return 0;
   }
   return tcb->pcb->pid;
}


