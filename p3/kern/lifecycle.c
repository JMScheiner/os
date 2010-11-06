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
#include <global_thread.h>
#include <reg.h>
#include <mm.h>
#include <kvm.h>
#include <vstring.h>
#include <loader.h>
#include <simics.h>
#include <thread.h>
#include <mode_switch.h>
#include <atomic.h>
#include <stub.h>
#include <cr.h>
#include <scheduler.h>
#include <string.h>
#include <mutex.h>
#include <cond.h>
#include <x86/asm.h>
#include <simics.h>
#include <types.h>
#include <debug.h>
#include <eflags.h>
#include <cr.h>
#include <seg.h>
#include <ecodes.h>
#include <region.h>
#include <console.h>
#include <thread.h>
#include <hashtable.h>

void *zombie_stack = NULL;
mutex_t zombie_stack_lock;
mutex_t wait_vanish_lock;

extern pcb_t *init_process;

/**
 * @brief Initialize the lifecycle handler data structures.
 */
void lifecycle_init() {
	mutex_init(&zombie_stack_lock);
	mutex_init(&wait_vanish_lock);
}

/**
 * @brief Handle the exec system call.
 *
 * @param reg The register state of the user upon calling exec.
 */
void exec_handler(volatile regstate_t reg) {
	char *arg_addr = (char *)SYSCALL_ARG(reg);
   char* execname;
   char** argvec;
	
   char execname_buf[MAX_NAME_LENGTH];
	char execargs_buf[MAX_TOTAL_LENGTH];
	char *args_ptr = execargs_buf;
	int total_bytes = 0;
	int argc;
   
	/* Verify that the arguments lie in valid memory. */
   if(v_memcpy((char*)&execname, arg_addr, sizeof(char*)) < sizeof(char*))
		RETURN(SYSCALL_INVALID_ARGS);
   
   if(v_memcpy((char*)&argvec, 
      arg_addr + sizeof(char*), sizeof(char**)) < sizeof(char**))
		RETURN(SYSCALL_INVALID_ARGS);
   
   pcb_t* pcb = get_pcb();

   if(pcb->thread_count > 1)
      RETURN(E_MULTIPLE_THREADS);
   
   if(v_strcpy((char*)execname_buf, execname, MAX_NAME_LENGTH) < 0) 
		RETURN(EXEC_INVALID_NAME);

   debug_print("exec", "Called with program %s", execname_buf);

	/* Loop over every srgument, copying it to the kernel stack. */
   for(argc = 0 ;; argc++, argvec++)
   {
      char* arg;
		if (total_bytes == MAX_TOTAL_LENGTH) 
			RETURN(EXEC_ARGS_TOO_LONG);
	   
      if(v_memcpy((char*)&arg, (char*)argvec, sizeof(char*)) < 0)
			RETURN(EXEC_INVALID_ARG);

      if(arg == NULL)
         break;
		
		int arg_len = v_strcpy(args_ptr, arg, MAX_TOTAL_LENGTH - total_bytes);
      if (arg_len < 0) 
			RETURN(EXEC_INVALID_ARG);
		
      debug_print("exec", "Arg %d is %s", argc, args_ptr);
		total_bytes += arg_len;
		args_ptr += arg_len;
	}
	
	int err;
	if ((err = elf_check_header(execname_buf)) != ELF_SUCCESS) {
		RETURN(err);
	}

	simple_elf_t elf_hdr;
	if ((err = elf_load_helper(&elf_hdr, execname_buf)) != ELF_SUCCESS) {
		RETURN(err);
	}
	
   /* Free user memory, user memory regions. */
   assert(pcb->regions);
   free_region_list(pcb);
   mm_free_user_space(pcb);

	if(initialize_memory(execname_buf, elf_hdr, pcb) <  0)
   {
      /* This is a tough one - we can't return to user space, since
       *  it's gone.... The "right-thing-to-do" is to reserve the 
       *  frames before we make the call into initialize_memory, which
       *  requires us to have some awareness of how much memory initialize
       *  memory will allocate....FIXME
       */
      assert(0); 
   }
	void *stack = copy_to_stack(argc, execargs_buf, total_bytes);

	unsigned int user_eflags = get_user_eflags();
	debug_print("exec", "Running %s", execname_buf);
	sim_reg_process((void*)pcb->dir_p, execname_buf);
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
	debug_print("thread_fork", "Called from process %p", pcb);
   new_tcb = initialize_thread(pcb);
   
   if(new_tcb == NULL)
      RETURN(E_NOMEM);
   
   newtid = new_tcb->tid;
	debug_print("thread_fork", "New tcb %p, thread_count = %d", new_tcb, pcb->thread_count);
   
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
   
   if(current_pcb->thread_count > 1)
      RETURN(E_MULTIPLE_THREADS);
   
   new_pcb = initialize_process(FALSE);
   if(new_pcb == NULL) goto fork_fail_pcb;
   
   new_pcb->regions = duplicate_region_list(current_pcb);
   if(new_pcb->regions == NULL) goto fork_fail_dup_regions;

   new_tcb = initialize_thread(new_pcb);
   if(new_tcb == NULL) goto fork_fail_tcb;

	debug_print("fork", "Parent pcb %p, tcb %p", current_pcb, current_tcb);
	debug_print("fork", "New pcb %p, tcb %p", new_pcb, new_tcb);
   
   newpid = new_pcb->pid;
  
   /* Duplicate the current address space in the new process. */
   if(mm_duplicate_address_space(new_pcb) < 0) goto fork_fail_dup;
   
   /* Arrange the new processes context for it's first context switch. */
   new_tcb->esp = arrange_fork_context(
      new_tcb->kstack, (regstate_t*)&reg, new_pcb->dir_p);
   
   atomic_add(&current_pcb->unclaimed_children, 1);
	mutex_lock(&current_pcb->child_lock);
	LIST_INSERT_AFTER(current_pcb->children, new_pcb, child_node);
	mutex_unlock(&current_pcb->child_lock);
   
   /* Register the first thread in the new TCB. */
   sim_reg_child(new_pcb->dir_p, current_pcb->dir_p);
   scheduler_register(new_tcb);
   RETURN(newpid);

fork_fail_dup: 
   free_thread_resources(new_tcb);
fork_fail_tcb:
fork_fail_dup_regions: 
   sfree(new_pcb->status, sizeof(status_t));
   free_process_resources(new_pcb);
fork_fail_pcb: 
   lprintf(" Failed to allocate new PCB in fork() ");
   RETURN(E_NOMEM);
}

/** 
* @brief Arranges a context we can jump to if we need to 
*  twiddle our thumbs, and plants it in the global TCB. 
*/
void arrange_global_context()
{
   void* esp;
   void (**ret_site)(void);
   tcb_t* tcb;
   
   tcb = global_tcb();
   esp = tcb->kstack;
   
   /* First give it a proper "iret frame" */
   /* Register contents do not matter. */
   regstate_t reg;
   reg.eip = (uint32_t)(loop_stub);
   reg.cs = SEGSEL_KERNEL_CS;
   reg.eflags = get_eflags() | EFL_IF;
   reg.esp = (uint32_t)tcb->kstack;
   reg.ss = SEGSEL_KERNEL_DS;
   
   esp -= sizeof(regstate_t);
   memcpy(esp, (void*)&reg, sizeof(regstate_t));
   
   /* Push the return address for a context switches ret */
   esp -= 4; 
   ret_site = esp;
   (*ret_site) = (pop_stub);
   
   /* Set up the context context_switch will popa off the stack. */
   esp -= sizeof(pusha_t);
   tcb->esp = esp;
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
	pcb->status->status = (int)SYSCALL_ARG(reg);
	debug_print("vanish", "Set status of pcb %p to %d", pcb, 
			pcb->status->status);
}

void thread_kill(char* error_message)
{
   putbytes(error_message, strlen(error_message));
   putbytes("\n", 1);
   
   pcb_t* pcb = get_pcb();
   pcb->status->status = STATUS_KILLED;
   MAGIC_BREAK;
   vanish_handler();
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
* @param reg Ignored.
*/
void vanish_handler()
{
	tcb_t *tcb = get_tcb();
	pcb_t *pcb = tcb->pcb;
	debug_print("vanish", "Thread %p from process %p", tcb, pcb);
	int remaining_threads = atomic_add(&pcb->thread_count, -1);
	if (remaining_threads == 1) {
		
		pcb_t *child;
		mutex_lock(&pcb->child_lock);
		LIST_FORALL(pcb->children, child, child_node) {
			child->parent = init_process;
		}
		mutex_unlock(&pcb->child_lock);
		
		mutex_lock(&wait_vanish_lock);
		pcb_t *parent = pcb->parent;
		if (parent != init_process)
			LIST_REMOVE(parent->children, pcb, child_node);
		debug_print("vanish", "Last thread, signalling %p", parent);

		status_t *status = pcb->status;
		mutex_lock(&parent->status_lock);
		status->next = parent->zombie_statuses;
		parent->zombie_statuses = status;
		mutex_unlock(&parent->status_lock);

		cond_signal(&parent->wait_signal);
		mutex_unlock(&wait_vanish_lock);
      
      /* Continue execution in the global address space. */
      set_cr3((int)global_pcb()->dir_p);
      free_process_resources(pcb);
	}


	mutex_lock(&tcb_table.lock);
	hashtable_remove(&tcb_table, tcb->tid);
	mutex_unlock(&tcb_table.lock);
	mutex_lock(&zombie_stack_lock);
	debug_print("vanish", "Freeing zombie %p", zombie_stack);
	if (zombie_stack) 
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
	debug_print("wait", "Called with status address %p", status_addr);
	if (status_addr != NULL && 
			!mm_validate_write(status_addr, sizeof(int))) {
		RETURN(SYSCALL_INVALID_ARGS);
	}

	pcb_t *pcb = get_pcb();
	debug_print("wait", "pcb = %p", pcb);
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
	debug_print("wait", "zombie child status = %p before cond_wait", 
			pcb->zombie_statuses);
	quick_lock();
	if (pcb->zombie_statuses == NULL) {
		cond_wait(&pcb->wait_signal);
	}
	else {
		quick_unlock();
	}

	debug_print("wait", "zombie child status = %p after cond_wait", 
			pcb->zombie_statuses);
	mutex_lock(&pcb->status_lock);
	status_t *status = pcb->zombie_statuses;
	assert(status != NULL);
	pcb->zombie_statuses = pcb->zombie_statuses->next;
	mutex_unlock(&pcb->status_lock);

	mutex_unlock(&pcb->waiter_lock);

	if (status_addr) {
		// There's nothing we can do if the copy fails, but don't crash. */
		v_memcpy((char *)status_addr, (char *)&status->status, sizeof(int));
	}
	int tid = status->tid;
	sfree(status, sizeof(status_t));
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

