
#include <process.h>
#include <mm.h>
#include <assert.h>
#include <hashtable.h>
#include <string.h>
#include <atomic.h>
#include <page.h>

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

static int allocate_region(char *start, char *end, int access_level) {
	int err;
   
   lprintf("Allocating region: start = %p, end = %p", start, end);

	if ((err = mm_new_pages((void *)start, (end - start + PAGE_SIZE - 1) / PAGE_SIZE, access_level)) != 0) {
		return err;
	}
	return 0;
}

static void initialize_region(const char *file, unsigned long offset, unsigned long len,
		unsigned long start, unsigned long end) {

   lprintf("Initializing region: file = %s, offset = %ld, len = %ld, start = %p, end = %p", 
      file, offset, len, (void*)start, (void*)end);
	getbytes(file, offset, len, (char *)start);
   MAGIC_BREAK;
	memset((char *)start + len, 0, end - start - len);
}

int initialize_memory(const char *file, simple_elf_t elf) {
	int err;
	if ((err = allocate_region((char *)elf.e_txtstart, 
				(char *)elf.e_datstart, PTENT_RO | PTENT_USER)) != 0) {
		return err;
	}
	initialize_region(file, elf.e_txtoff, elf.e_txtlen, elf.e_txtstart, elf.e_rodatstart);
	initialize_region(file, elf.e_rodatoff, elf.e_rodatlen, elf.e_rodatstart, elf.e_datstart);

	if ((err = allocate_region((char *)elf.e_datstart, 
				(char *)elf.e_datstart + elf.e_datlen + elf.e_bsslen, PTENT_RW | PTENT_USER)) != 0) {
		return err;
	}
	initialize_region(file, elf.e_datoff, elf.e_datlen, elf.e_datstart, 
			elf.e_datstart + elf.e_datlen + elf.e_bsslen);
	if ((err = allocate_region((char *)(USER_STACK_BASE - PAGE_SIZE), 
				(char *)USER_STACK_BASE, PTENT_RW | PTENT_USER)) != 0) {
		return err;
	}
	return 0;
}
