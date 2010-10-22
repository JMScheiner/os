
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

/**
 * @brief Next pid to assign to a process.
 */
static int next_pid = 0xc0de0000;

DEFINE_HASHTABLE(pcb_table_t, int, pcb_t *);

/**
 * @brief Hashtable mapping pids to pcbs.
 */
pcb_table_t pcb_table;

/**
 * @brief Mutual exclusion lock for pcb_table.
 */
mutex_t pcb_table_lock;

/**
 * @brief Initialize the pcb_table.
 */
void init_process_table(void)
{
   mutex_init(&pcb_table_lock);
	STATIC_INIT_HASHTABLE(pcb_table_t, pcb_table, default_hash);
}

/**
 * @brief Get the pcb of the currently running process.
 *
 * @return The pcb.
 */
pcb_t* get_pcb()
{
   tcb_t* tcb = get_tcb();
   assert(tcb);
   return tcb->pcb;
}

pcb_t* initialize_first_process() 
{
   pcb_t* pcb = (pcb_t*) malloc(sizeof(pcb_t));
	pcb->pid = atomic_add(&next_pid, 1);
	pcb->ppid = 0;
	pcb->thread_count = 0;
   pcb->regions = NULL;
   
	pcb->page_directory = mm_new_directory();
   lprintf("Sanity check: pcb = %p, page_directory = %p", 
      pcb, pcb->page_directory);
	mutex_init(&pcb->lock);
	mutex_init(&pcb->directory_lock);
	mutex_init(&pcb->region_lock);
   
   mutex_lock(&pcb_table_lock);
   HASHTABLE_PUT(pcb_table_t, pcb_table, pcb->pid, pcb);
   mutex_unlock(&pcb_table_lock);
	
   return pcb;
}

pcb_t* initialize_process() 
{
   pcb_t* pcb = (pcb_t*) malloc(sizeof(pcb_t));
	pcb->pid = atomic_add(&next_pid, 1);
	pcb->ppid = get_pid();
	pcb->thread_count = 0;
   pcb->regions = NULL;
   
	pcb->page_directory = mm_new_directory();
	mutex_init(&pcb->lock);
	mutex_init(&pcb->directory_lock);
	mutex_init(&pcb->region_lock);
   
   mutex_lock(&pcb_table_lock);
   HASHTABLE_PUT(pcb_table_t, pcb_table, pcb->pid, pcb);
   mutex_unlock(&pcb_table_lock);
	
   return pcb;
}

int get_pid() {
	tcb_t *tcb = get_tcb();
	if (tcb == NULL) {
		return 0;
	}
	return tcb->pcb->pid;
}

static void initialize_region(const char *file, unsigned long offset, unsigned long len,
		unsigned long start, unsigned long end) 
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
   allocate_region(
      (char*)elf.e_datstart, (char*)elf.e_datstart + elf.e_datlen + elf.e_bsslen, 
      PTENT_RW | PTENT_USER,  dat_fault, pcb);
      
   //Allocate bss region.
   // TODO Keep a global "zero" read only page for ZFOD regions (like bss).
   allocate_region((char*)elf.e_datstart + elf.e_datlen, 
      elf.e_datstart + elf.e_datlen + elf.e_bsslen, PTENT_RW | PTENT_USER | PTENT_ZFOD, 
      bss_fault, pcb);
   
   // Allocate stack region (same for all processes).
   allocate_stack_region(pcb);

   initialize_region(file, elf.e_txtoff, elf.e_txtlen, elf.e_txtstart, elf.e_rodatstart);
	initialize_region(file, elf.e_rodatoff, elf.e_rodatlen, elf.e_rodatstart, elf.e_datstart);
	initialize_region(file, elf.e_datoff, elf.e_datlen, elf.e_datstart, 
      elf.e_datstart + elf.e_datlen + elf.e_bsslen);
			
	return 0;
}
