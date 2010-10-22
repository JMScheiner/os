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
#include <validation.h>
#include <loader.h>
#include <simics.h>
#include <thread.h>
#include <mode_switch.h>
#include <atomic.h>
#include <pop_stub.h>
#include <cr.h>
#include <scheduler.h>
#include <string.h>

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
	assert(initialize_memory(execname_buf, elf_hdr, get_pcb()) == 0);
	void *stack = copy_to_stack(argc, execargs_buf, total_bytes);

	unsigned int user_eflags = get_user_eflags();
	lprintf("Running %s", execname_buf);
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
   tcb_t* current_tcb, *new_tcb;

   pcb = get_pcb();
   current_tcb = get_tcb();
   
   new_tcb = initialize_thread(pcb);
   newtid = new_tcb->tid;
   atomic_add(&pcb->thread_count, 1);
   
   new_tcb->esp = arrange_fork_context(
      new_tcb->kstack, (regstate_t*)&reg, (void*)get_cr3());
   
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
   pcb_t *current_pcb, *new_pcb; 
   tcb_t *current_tcb, *new_tcb;

   current_pcb = get_pcb();
   current_tcb = get_tcb();
   
   new_pcb = initialize_process();
   
   new_tcb = initialize_thread(new_pcb);
   new_pcb->thread_count = 1;
   newpid = new_pcb->pid;
  
   /* Duplicate the current address space in the new process. */
   mm_duplicate_address_space(new_pcb);
   
   /* Arrange the new processes context for it's first context switch. */
   new_tcb->esp = arrange_fork_context(
      new_tcb->kstack, (regstate_t*)&reg, new_pcb->page_directory);
   
   /* Register the first thread in the new TCB. */
   sim_reg_child(new_pcb->page_directory, current_pcb->page_directory);
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
* @param page_directory The page directory the new thread will execute with.
* 
* @return The stack pointer to context switch to. Should be installed into 
*  the new threads TCB so the context switcher knows where to jump.
*/
void* arrange_fork_context(void* esp, regstate_t* reg, void* page_directory)
{
   /* First give it a proper "iret frame" */
   esp -= sizeof(regstate_t);
   memcpy(esp, (void*)reg, sizeof(regstate_t));
   
   /* Set eax to zero for the iret from either thread_fork or fork. */
   regstate_t* new_reg = (regstate_t*)esp;
   new_reg->eax = 0;
   
   /* Push the return address for context switches ret */
   esp -= 4; 
   void (**ret_site)(void);
   ret_site = esp;

   (*ret_site) = (pop_stub);
   
   /* Set up the context context_switch will popa off the stack. */
   esp -= sizeof(pusha_t);
   pusha_t* pusha = (pusha_t*)esp;
   pusha->eax = (unsigned long)page_directory;
   return esp;
}

/** 
* @brief Sets the exit status of the current task.
* 
* @param reg The register state on entry and exit of the handler. 
*/
void set_status_handler(volatile regstate_t reg)
{
	lprintf("Ignoring set_status ");
	MAGIC_BREAK;
   //TODO
}

/** 
* @brief Terminates execution of the calling thread "immediately."
*
*  If the invoking thread is the last thread in its task, the kernel deallocates 
*     all resources in use by the task and makes the exit status of the task
*     available to the parent task (the task which created this task using fork())
*     via wait().
*
*  If the parent task is no longer running, exit status of the task is made available to 
*     the kernel-launched "init" task instead. 
* 
* @param reg The register state on entry and exit of the handler. 
*/
void vanish_handler(volatile regstate_t reg)
{
	lprintf("Ignoring vanish");
	MAGIC_BREAK;
   //TODO
}

/** 
* @brief Collects the exit status of a task and stores it in the integer referenced
*  in %esi.
* 
* @param reg The register state on entry and exit of the handler. 
*/
void wait_handler(volatile regstate_t reg)
{
	lprintf("Ignoring wait");
	MAGIC_BREAK;
   //TODO
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

