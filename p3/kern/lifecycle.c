/** 
* @file lifecycle.c
*
* @brief Implementation of system calls mentioned in the 
*  lifecycle section of the "Pebbles" kernel specification. 
*
* @author Tim Wilson
* @author Justin Scheiner
* @date 2010-10-20
*/

#include <lifecycle.h>
#include <reg.h>
#include <mm.h>
#include <kvm.h>
#include <validation.h>
#include <loader.h>
#include <debug.h>
#include <thread.h>
#include <mode_switch.h>
#include <atomic.h>
#include <pop_stub.h>
#include <cr.h>
#include <scheduler.h>
#include <string.h>
#include <mutex.h>
#include <cond.h>
#include <x86/asm.h>
#include <simics.h>
#include <types.h>

void *zombie_stack = NULL;
mutex_t zombie_stack_lock;

extern pcb_t *init_process;

/**
 * @brief Initialize the lifecycle handler data structures.
 */
void lifecycle_init() {
	mutex_init(&zombie_stack_lock);
}

/**
 * @brief Handle the exec system call.
 *
 * @param reg The register state of the user upon calling exec.
 */
void exec_handler(volatile regstate_t reg) {
	char *arg_addr = (char *)SYSCALL_ARG(reg);
	int argc;
	char execname_buf[MAX_NAME_LENGTH];
	char execargs_buf[MAX_TOTAL_LENGTH];
	char *name_ptr = execname_buf;
	char *args_ptr = execargs_buf;
	int total_bytes = 0;

	/* Verify that the arguments lie in valid memory. */
	if (!mm_validate(arg_addr) || !mm_validate(arg_addr + sizeof(void *))) {
		RETURN(EXEC_INVALID_ARGS);
	}

	/* TODO Check if there is more than one thread. */
	char *execname = *(char **)arg_addr;
	char **argvec = *(char ***)(arg_addr + sizeof(char *));
	if (v_strcpy(name_ptr, execname, MAX_NAME_LENGTH) < 0) {
		RETURN(EXEC_INVALID_NAME);
	}

	/* Loop over every srgument, copying it to the kernel stack. */
	SAFE_LOOP(argvec, argc, MAX_TOTAL_LENGTH) {
		if (total_bytes == MAX_TOTAL_LENGTH) {
			RETURN(EXEC_ARGS_TOO_LONG);
		}
		if (*argvec == NULL) {
			break;
		}
		char *arg = *argvec;
		int arg_len = v_strcpy(args_ptr, arg, MAX_TOTAL_LENGTH - total_bytes);
		if (arg_len < 0) {
			RETURN(EXEC_INVALID_ARG);
		}
		total_bytes += arg_len;
		args_ptr += arg_len;
	}
	
	//load_new_task(execname_buf, argc, execargs_buf, total_bytes);
	int err;
	if ((err = elf_check_header(execname_buf)) != ELF_SUCCESS) {
		RETURN(err);
	}

	// TODO checking and loading the elf header should happen separately so exec
	// can do it before freeing the current process.
	simple_elf_t elf_hdr;
	if ((err = elf_load_helper(&elf_hdr, execname_buf)) != ELF_SUCCESS) {
		RETURN(err);
	}
	
	/* TODO Free user memory regions. We should also check that we get a valid
	 * elf header before freeing. */
   
	// TODO This probably shouldn't be an assert.
   pcb_t* pcb = get_pcb();
	assert(initialize_memory(execname_buf, elf_hdr, pcb) == 0);
	void *stack = copy_to_stack(argc, execargs_buf, total_bytes);

	unsigned int user_eflags = get_user_eflags();
	debug_print("exec", "Running %s", execname_buf);
	sim_reg_process((void*)get_cr3(), execname_buf);
	mode_switch(get_tcb()->esp, stack, user_eflags, (void *)elf_hdr.e_entry);
	// Never get here
	assert(0);
}

/** 
* @brief Generates a new thread in the current address space. 
*  Explicitly:
*     1. Allocates a new kernel stack. 
*     2. Initializes a new TCB struct. 
*     3. Arranges the proper context for a context switch to occur.
*     4. Adds the new thread to the scheduler queue.
* 
* @param reg The register state put on the stack by INT and PUSHA
*/
void thread_fork_handler(volatile regstate_t reg)
{
   unsigned long newtid;
   pcb_t* pcb;
   tcb_t* new_tcb;

   pcb = get_pcb();
   new_tcb = initialize_thread(pcb);
   newtid = new_tcb->tid;
   atomic_add(&pcb->thread_count, 1);
   
   new_tcb->esp = arrange_fork_context(
      new_tcb->kstack, (regstate_t*)&reg, (void*)pcb->dir_p);
   
   /* TODO Does something need to happen here for user level debugging? */
   scheduler_register(new_tcb);
   RETURN(newtid);
}

/** 
* @brief Generates a new thread with a deep copy of the current address space. 
*  Explicitly:
*     1. Allocates a new page directory and PCB.
*     2. Allocates a new kernel stack and TCB.
*     3. Duplicates the current address space in the new page directory. 
*     4. Arranges the proper context for a context switch to occur.
*     5. Adds the new thread to the scheduler queue.
* 
* @param reg The register state put on the stack by INT and PUSHA
*/
void fork_handler(volatile regstate_t reg)
{
   unsigned long newpid; 
   pcb_t *new_pcb; 
   tcb_t *new_tcb;

   tcb_t *current_tcb = get_tcb();
   pcb_t *current_pcb = current_tcb->pcb;
   
   new_pcb = initialize_process(FALSE);
   new_tcb = initialize_thread(new_pcb);
   new_pcb->thread_count = 1;
   newpid = new_pcb->pid;
	 atomic_add(&current_pcb->unclaimed_children, 1);
  
   /* Duplicate the current address space in the new process. */
   mm_duplicate_address_space(new_pcb);
   
   /* Arrange the new processes context for it's first context switch. */
   new_tcb->esp = arrange_fork_context(
      new_tcb->kstack, (regstate_t*)&reg, new_pcb->dir_p);
   
   /* Register the first thread in the new TCB. */
   sim_reg_child(new_pcb->dir_p, get_pcb()->dir_p);
   scheduler_register(new_tcb);
   
   RETURN(newpid);
}

/** 
* @brief Arranges the context for the first invocation of context_switch to 
*  a new thread. 
* 
* @param esp The initial stack pointer to build the context on. 
*     - Generally the address of a new kernel stack. 
*
* @param reg The register state on entry to the new process. 
*     - This function is responsible for setting %eax to 0 for new threads.
*
* @param dir The page directory the new thread will execute with.
* 
* @return The stack pointer to context switch to. Should be installed into 
*  the new threads TCB so the context switcher knows where to jump.
*/
void* arrange_fork_context(void* esp, regstate_t* reg, void* dir)
{
   /* First give it a proper "iret frame" */
   esp -= sizeof(regstate_t);
   memcpy(esp, (void*)reg, sizeof(regstate_t));
   
   /* Set eax to zero for the iret from either thread_fork or fork. */
   regstate_t* new_reg = (regstate_t*)esp;
   new_reg->pusha.eax = 0;
   
   /* Push the return address for context switches ret */
   esp -= 4; 
   void (**ret_site)(void);
   ret_site = esp;

   (*ret_site) = (pop_stub);
   
   /* Set up the context context_switch will popa off the stack. */
   esp -= sizeof(pusha_t);
   pusha_t* pusha = (pusha_t*)esp;
   pusha->eax = (unsigned long)dir;
   return esp;
}

/** 
* @brief Sets the exit status of the current task.
* 
* @param reg The register state on entry and exit of the handler. 
*/
void set_status_handler(volatile regstate_t reg)
{
	pcb_t *pcb = get_pcb();
	pcb->status.status = (int)SYSCALL_ARG(reg);
}

/** 
* @brief Terminates execution of the calling thread "immediately."
*
*  If the invoking thread is the last thread in its task, the kernel 
*  deallocates all resources in use by the task and makes the exit status 
*  of the task available to the parent task (the task which created this 
*  task using fork()) via wait().
*
*  If the parent task is no longer running, exit status of the task is 
*  made available to the kernel-launched "init" task instead. 
* 
* @param reg The register state on entry and exit of the handler. 
*/
void vanish_handler(volatile regstate_t reg)
{
	tcb_t *tcb = get_tcb();
	pcb_t *pcb = tcb->pcb;
	int remaining_threads = atomic_add(&pcb->thread_count, -1);
	if (remaining_threads == 1) {
		// We are the last thread in our process
		//region_t *region = pcb->regions;
		// Free regions
		pcb_t *parent = pcb->parent;
		if (parent == NULL) {
			parent = init_process;
		}
		status_t *status = &pcb->status;
		mutex_lock(&parent->status_lock);
		status->next = parent->zombie_statuses;
		parent->zombie_statuses = status;
		mutex_unlock(&parent->status_lock);
		cond_signal(&parent->wait_signal);
	}
	
	mutex_lock(&zombie_stack_lock);
	kvm_free_page(zombie_stack);
	zombie_stack = tcb;
	scheduler_die(&zombie_stack_lock);
	assert(FALSE);
}

/** 
* @brief Collects the exit status of a task and stores it in the 
* integer referenced in %esi.
* 
* @param reg The register state on entry and exit of the handler. 
*/
void wait_handler(volatile regstate_t reg)
{
	int *status_addr = (int *)SYSCALL_ARG(reg);
	if (!mm_validate_write(status_addr, sizeof(int))) {
		RETURN(WAIT_INVALID_ARGS);
	}

	pcb_t *pcb = get_pcb();
	mutex_lock(&pcb->check_waiter_lock);
	if (pcb != init_process && pcb->unclaimed_children == 0) {
		/* There are threads waiting on every child process. We will not be 
		 * able to collect a status so return immediately. */
		mutex_unlock(&pcb->check_waiter_lock);
		RETURN(WAIT_NO_CHILDREN);
	}

	/* The unclaimed_children field will be meaningless for init_process. */
	atomic_add(&pcb->unclaimed_children, -1);
	assert(pcb == init_process || pcb->unclaimed_children >= 0);
	mutex_unlock(&pcb->check_waiter_lock);

	mutex_lock(&pcb->waiter_lock);
	disable_interrupts();
	if (pcb->zombie_statuses == NULL) {
		cond_wait(&pcb->wait_signal);
	}
	enable_interrupts();

	mutex_lock(&pcb->status_lock);
	status_t *status = pcb->zombie_statuses;
	assert(status != NULL);
	pcb->zombie_statuses = pcb->zombie_statuses->next;
	mutex_unlock(&pcb->status_lock);

	mutex_unlock(&pcb->waiter_lock);

	*status_addr = status->status;
	int tid = status->tid;
	free(pcb);
	RETURN(tid);
}

/** 
* @brief Causes all threads of a task to vanish().  The exit status of the task, as 
*  returned via wait(), will be the value of the status parameter in %esi. 
* 
* @param reg The register state on entry and exit of the handler. 
*/
void task_vanish_handler(volatile regstate_t reg)
{
	lprintf("Ignoring task_vanish");
	MAGIC_BREAK;
   //TODO
}

