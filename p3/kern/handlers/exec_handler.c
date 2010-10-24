
#include <reg.h>
#include <mm.h>
#include <validation.h>
#include <loader.h>
#include <debug.h>
#include <thread.h>
#include <mode_switch.h>


/**
 * @brief Return with the given value. Useful for trap handlers to set the eax
 * return value.
 *
 * @param ret The value to return.
 */
#define RETURN(ret) \
	do { \
		reg.eax = ret; \
		return; \
	} while (0)

/** @brief Maxixmum allowed size of the kernel stack to use for exec arguments.
 * Should be some constant fraction of the kernel stack. */
#define MAX_TOTAL_LENGTH ((KERNEL_STACK_SIZE * PAGE_SIZE) / 2)

#define MAX_NAME_LENGTH 127

/**
 * @brief Error code indicating the arguments to exec are not in the user's
 * memory region.
 */
#define EXEC_INVALID_ARGS -1
/**
 * @brief Error code indicating one of the string arguments is not in the
 * user's memory region.
 */
#define EXEC_INVALID_ARG -2
/**
 * @brief Error code indicating that the total size of the arguments to exec is
 * too large.
 */
#define EXEC_ARGS_TOO_LONG -3

#define EXEC_INVALID_NAME -4

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
	debug_print_exec("Running %s", execname_buf);
	mode_switch(get_tcb()->esp, stack, user_eflags, (void *)elf_hdr.e_entry);
	// Never get here
	assert(0);
}

