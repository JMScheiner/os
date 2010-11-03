
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

/**
 * @brief Next pid to assign to a process.
 */
static int next_pid = 1;

/** @brief pcb of the init process. */
pcb_t *init_process;

//DEFINE_HASHTABLE(pcb_table_t, int, pcb_t *);

/** @brief Hashtable mapping pids to pcbs. */
//pcb_table_t pcb_table;

/** @brief Mutual exclusion lock for pcb_table. */
//mutex_t pcb_table_lock;

/**
 * @brief Initialize the pcb_table.
 */
void init_process_table(void)
{
	//mutex_init(&pcb_table_lock);
	//mutex_init(&status_table_lock);
	//STATIC_INIT_HASHTABLE(pcb_table_t, pcb_table, default_hash, 
	//		&pcb_table_lock);
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
	pcb_t* pcb = (pcb_t*) smalloc(sizeof(pcb_t));
	// TODO Do something smarter
	assert(pcb);
   pcb->pid = atomic_add(&next_pid, 1);
	if (first_process) {
		pcb->parent = NULL;
		init_process = pcb;
	}
	else {
		pcb->parent = get_pcb();
	}
	pcb->thread_count = 0;
	pcb->regions = NULL;
	pcb->status = (status_t *)smalloc(sizeof(status_t));
	// TODO Do something smarter
	assert(pcb->status);
	pcb->status->status = 0;
	INIT_LIST(pcb->children);
	pcb->unclaimed_children = 0;
   pcb->zombie_statuses = NULL;
   pcb->sanity_constant = PCB_SANITY_CONSTANT;
   
	kvm_new_directory(pcb);
	mutex_init(&pcb->lock);
	mutex_init(&pcb->directory_lock);
	mutex_init(&pcb->kvm_lock);
	mutex_init(&pcb->region_lock);
	mutex_init(&pcb->status_lock);
	mutex_init(&pcb->waiter_lock);
	mutex_init(&pcb->check_waiter_lock);
	mutex_init(&pcb->child_lock);

	cond_init(&pcb->wait_signal);
   
	//HASHTABLE_PUT(pcb_table_t, pcb_table, pcb->pid, pcb);
	
	return pcb;
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
	//int err;
   
   // Allocate text region. FIXME Allocate text and rodata together.
   allocate_region((char*)elf.e_txtstart, (char*)elf.e_rodatstart, 
      PTENT_RO | PTENT_USER, txt_fault, pcb);
      
   // Allocate rodata region.
   allocate_region(
      (char*)elf.e_rodatstart, elf.e_rodatstart + (char*)elf.e_rodatlen, 
      PTENT_RO | PTENT_USER,  rodata_fault, pcb);
   
   //Allocate data region.
   allocate_region((char*)elf.e_datstart, 
      (char*)elf.e_datstart + elf.e_datlen + elf.e_bsslen, 
      PTENT_RW | PTENT_USER,  dat_fault, pcb);
      
   //Allocate bss region.
   // TODO Keep a global "zero" read only page for ZFOD regions (like bss).
   allocate_region((char*)elf.e_datstart + elf.e_datlen, 
      elf.e_datstart + elf.e_datlen + elf.e_bsslen, 
      PTENT_RW | PTENT_USER | PTENT_ZFOD, bss_fault, pcb);
      
   
   // Allocate stack region (same for all processes).
   allocate_stack_region(pcb);

   initialize_region(file, elf.e_txtoff, elf.e_txtlen, 
      elf.e_txtstart, elf.e_rodatstart);
      
   initialize_region(file, elf.e_rodatoff, elf.e_rodatlen, 
      elf.e_rodatstart, elf.e_datstart);
	
   initialize_region(file, elf.e_datoff, elf.e_datlen, elf.e_datstart, 
      elf.e_datstart + elf.e_datlen + elf.e_bsslen);
			
	return 0;
}
