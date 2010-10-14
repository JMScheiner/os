
#include <process.h>
#include <mm.h>
#include <assert.h>
#include <hashtable.h>
#include <string.h>
#include <atomic.h>
#include <page.h>
#include <region.h>
#include <pagefault.h>

static int next_pid = 0;

DEFINE_HASHTABLE(pcb_table_t, int, pcb_t *);

pcb_table_t pcb_table;

void init_process_table(void) {
	STATIC_INIT_HASHTABLE(pcb_table_t, pcb_table, default_hash);
}

int initialize_process(pcb_t *pcb) {
	assert(pcb);
	pcb->pid = atomic_add(&next_pid, 1);
	pcb->ppid = get_pid();
	pcb->thread_count = 0;
   pcb->regions = NULL;
   
   lprintf("allocating new directory.");
	pcb->page_directory = mm_new_directory();
   lprintf("done");
	pcb->thread = NULL;
	//mutex_init(&pcb->lock);
	return 0;
}

int get_pid() {
   lprintf("get_pid");
	tcb_t *tcb = get_tcb();
	if (tcb == NULL) {
		return 0;
	}
	return tcb->pid;
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
   
   // Allocate text region.
   allocate_region((char*)elf.e_txtstart, (char*)elf.e_datstart, 
      PTENT_RO | PTENT_USER, txt_fault, txt_free, pcb);
      
   // Allocate rodata region.
   allocate_region((char*)elf.e_rodatstart, elf.e_rodatstart + (char*)elf.e_rodatlen, 
      PTENT_RO | PTENT_USER,  rodata_fault, rodata_free, pcb);
   
   //Allocate data region.
   allocate_region((char*)elf.e_datstart, (char*)elf.e_datstart + elf.e_datlen + elf.e_bsslen, 
      PTENT_RW | PTENT_USER,  dat_fault, dat_free, pcb);
      
   //Allocate bss region.
   // TODO Keep a global "zero" read only page for ZFOD regions (like bss).
   allocate_region((char*)elf.e_datstart + elf.e_datlen, 
      elf.e_datstart + elf.e_datlen + elf.e_bsslen, PTENT_RW | PTENT_USER | PTENT_ZFOD, 
      bss_fault, bss_free, pcb);
   
   // Allocate stack region.
   allocate_region((char *)(USER_STACK_BASE - PAGE_SIZE), 
				(char *)USER_STACK_BASE, PTENT_RW | PTENT_USER, stack_fault, stack_free);

	initialize_region(file, elf.e_txtoff, elf.e_txtlen, elf.e_txtstart, elf.e_rodatstart);
	initialize_region(file, elf.e_rodatoff, elf.e_rodatlen, elf.e_rodatstart, elf.e_datstart);
	initialize_region(file, elf.e_datoff, elf.e_datlen, elf.e_datstart, 
      elf.e_datstart + elf.e_datlen + elf.e_bsslen);
			
	return 0;
}
