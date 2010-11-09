
#include <process.h>
#include <mm.h>
#include <assert.h>
#include <hashtable.h>
#include <string.h>
#include <atomic.h>
#include <page.h>
#include <region.h>
#include <thread.h>
#include <pagefault.h>
#include <mutex.h>
#include <global_thread.h>
#include <cond.h>
#include <kvm.h>
#include <ecodes.h>
#include <simics.h>
#include <malloc_wrappers.h>

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
	mutex_destroy(&pcb->vanish_lock);
	mutex_destroy(&pcb->waiter_lock);
	mutex_destroy(&pcb->check_waiter_lock);
	mutex_destroy(&pcb->child_lock);
	cond_destroy(&pcb->wait_signal);
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
   pcb->zombie_statuses = NULL;
   pcb->sanity_constant = PCB_SANITY_CONSTANT;
   
	mutex_init(&pcb->directory_lock);
	mutex_init(&pcb->region_lock);
	mutex_init(&pcb->status_lock);
	mutex_init(&pcb->vanish_lock);
	mutex_init(&pcb->waiter_lock);
	mutex_init(&pcb->check_waiter_lock);
	mutex_init(&pcb->child_lock);

	cond_init(&pcb->wait_signal);
   
	return pcb;

fail_status: 
   mm_free_address_space(pcb);
fail_new_directory:
   sfree(pcb, sizeof(pcb_t));
fail_pcb: 
   return NULL;
}

int get_pid() {
	tcb_t *tcb = get_tcb();
	if (tcb == NULL) {
		return 0;
	}
	return tcb->pcb->pid;
}

static void initialize_region(const char *file, unsigned long offset, 
		unsigned long len, unsigned long start, unsigned long end) 
{
	getbytes(file, offset, len, (char *)start);
	memset((char *)start + len, 0, end - start - len);
}

int initialize_memory(const char *file, simple_elf_t elf, pcb_t* pcb) 
{
   // Allocate text region. FIXME Allocate text and rodata together.
   if(allocate_region((char*)elf.e_txtstart, (char*)elf.e_rodatstart, 
      PTENT_RO | PTENT_USER, txt_fault, pcb) < 0) goto fail_init_mem;
      
   // Allocate rodata region.
   if(allocate_region(
      (char*)elf.e_rodatstart, elf.e_rodatstart + (char*)elf.e_rodatlen, 
      PTENT_RO | PTENT_USER,  rodata_fault, pcb) < 0) goto fail_init_mem;
   
   //Allocate data region.
   if(allocate_region((char*)elf.e_datstart, 
      (char*)elf.e_datstart + elf.e_datlen + elf.e_bsslen, 
      PTENT_RW | PTENT_USER,  dat_fault, pcb) < 0) goto fail_init_mem;
      
   //Allocate bss region.
   // TODO Keep a global "zero" read only page for ZFOD regions (like bss).
   if(allocate_region((char*)elf.e_datstart + elf.e_datlen, 
      elf.e_datstart + elf.e_datlen + elf.e_bsslen, 
      PTENT_RW | PTENT_USER | PTENT_ZFOD, bss_fault, pcb) < 0) goto fail_init_mem;
      
   
   // Allocate stack region (same for all processes).
   if(allocate_stack_region(pcb) < 0) goto fail_init_mem;

   initialize_region(file, elf.e_txtoff, elf.e_txtlen, 
      elf.e_txtstart, elf.e_rodatstart);
      
   initialize_region(file, elf.e_rodatoff, elf.e_rodatlen, 
      elf.e_rodatstart, elf.e_datstart);
	
   initialize_region(file, elf.e_datoff, elf.e_datlen, elf.e_datstart, 
      elf.e_datstart + elf.e_datlen + elf.e_bsslen);
			
	return E_SUCCESS;

fail_init_mem:
   free_region_list(pcb);
   mm_free_user_space(pcb);
   return E_FAIL;
}


