
#include <reg.h>
#include <mm.h>
#include <validation.h>

#define RETURN(ret) \
	do { \
		reg.eax = ret; \
		return; \
	} while (0)

#define MAX_TOTAL_LENGTH ((KERNEL_STACK_SIZE * PAGE_SIZE) / 2)
#define EXEC_ARGS 2

#define EXEC_INVALID_ARGS -1
#define EXEC_ARGS_TOO_LONG -2

/**
 * @brief Handle the exec system call.
 *
 * @param reg The register state of the user upon calling exec.
 */
void exec_handler(volatile regstate_t reg) {
	char *arg_addr = (char *)SYSCALL_ARG(reg);
	int argc;
	int execname_len;
	char[MAX_TOTAL_LENGTH] buf;
	char *ptr = buf;

	/* TODO Check if there is more than one thread. */

	if (!mm_validate(arg_addr) || !mm_validate(arg_addr + sizeof(void *))) {
		RETURN(EXEC_INVALID_ARGS);
	}

	char *execname = *(char **)arg_addr;
	char **argvec = *(char ***)(arg_addr + sizeof(char *));
	int total_bytes += v_strcpy(ptr, execname, MAX_TOTAL_LENGTH - total_bytes);
	ptr += total_bytes;
	
	SAFE_LOOP(argvec, sizeof(char *), argc, MAX_TOTAL_LENGTH) {
		if (total_bytes == MAX_TOTAL_LENGTH) {
			RETURN(EXEC_ARGS_TOO_LONG);
		}
		if (*argvec == NULL) {
			break;
		}
		char *arg = *argvec;
		int arg_len = v_strcpy(ptr, arg, MAX_TOTAL_LENGTH - total_bytes);
		total_bytes += arg_len;
		ptr += arg_len;
	}

	/* execname is an argument too */
	argc++;

	/* TODO Free user memory regions. */

	load_new_task(argc, buf, total_bytes);
}

