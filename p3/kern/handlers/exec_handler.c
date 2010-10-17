
#include <reg.h>
#include <mm.h>
#include <validation.h>
#include <loader.h>
#include <simics.h>

#define RETURN(ret) \
	do { \
		reg.eax = ret; \
		return; \
	} while (0)

#define MAX_TOTAL_LENGTH ((KERNEL_STACK_SIZE * PAGE_SIZE) / 2)
#define EXEC_ARGS 2

#define EXEC_INVALID_ARGS -1
#define EXEC_INVALID_ARG -2
#define EXEC_ARGS_TOO_LONG -3

/**
 * @brief Handle the exec system call.
 *
 * @param reg The register state of the user upon calling exec.
 */
void exec_handler(volatile regstate_t reg) {
	char *arg_addr = (char *)SYSCALL_ARG(reg);
	int argc;
	char buf[MAX_TOTAL_LENGTH];
	char *ptr = buf;


	if (!mm_validate(arg_addr) || !mm_validate(arg_addr + sizeof(void *))) {
		RETURN(EXEC_INVALID_ARGS);
	}

	/* TODO Check if there is more than one thread. */
	char *execname = *(char **)arg_addr;
	char **argvec = *(char ***)(arg_addr + sizeof(char *));
	int total_bytes = v_strcpy(ptr, execname, MAX_TOTAL_LENGTH);
	MAGIC_BREAK;

	if (total_bytes < 0) {
		RETURN(EXEC_INVALID_ARG);
	}
	ptr += total_bytes;
	
	SAFE_LOOP(argvec, argc, MAX_TOTAL_LENGTH) {
		if (total_bytes == MAX_TOTAL_LENGTH) {
			RETURN(EXEC_ARGS_TOO_LONG);
		}
		if (*argvec == NULL) {
			break;
		}
		char *arg = *argvec;
		int arg_len = v_strcpy(ptr, arg, MAX_TOTAL_LENGTH - total_bytes);
		if (arg_len < 0) {
			RETURN(EXEC_INVALID_ARG);
		}
		total_bytes += arg_len;
		ptr += arg_len;
	}

	/* execname is an argument too */
	argc++;

	/* TODO Free user memory regions. */

	load_new_task(argc, buf, total_bytes);
}

