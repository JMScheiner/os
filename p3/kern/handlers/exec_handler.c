
#include <reg.h>
#include <mm.h>

/**
 * @brief Perform a checked loop over a region of memory
 *
 * @param addr The variable that will loop over the memory region
 * @param inc The number of bytes to increment the address by on each iteration
 *            of the loop.
 * @param cntr A variable counting the number of iterations performed by the
 *             loop. Need not be initialized.
 * @param max The maximum number of iterations to perform.
 */
#define SAFE_LOOP(addr, inc, cntr, max) \
	for ((cntr) = 0 ; \
			      (cntr) < (max) \
			   && ((cntr) != 0) \
			   && ((SAME_PAGE(addr, addr - inc)) \
			   || mm_validate(addr)) \
			; (cntr)++, \
				((addr)) += (inc))

#define RETURN(ret) \
	do { \
		reg.eax = ret; \
		return; \
	} while (0)

#define MAX_NAME_LENGTH 127
#define MAX_ARGUMENTS 127
#define MAX_ARG_LENGTH 127
#define EXEC_ARGS 2

#define EXEC_INVALID_ARGS -1
#define EXEC_EXE_TOO_LONG -2
#define EXEC_ARGS_TOO_LONG -3
#define EXEC_ARG_TOO_LONG -4

/**
 * @brief Handle the exec system call.
 *
 * @param reg The register state of the user upon calling exec.
 */
void exec_handler(volatile regstate_t reg) {
	void *arg_addr = (void *)SYSCALL_ARG(reg);
	int argc;
	int execname_len;

	if (!mm_validate(arg_addr) || !mm_validate(arg_addr + sizeof(void *))) {
		RETURN(EXEC_INVALID_ARGS);
	}

	char *execname = *(char **)arg_addr;
	SAFE_LOOP(execname, sizeof(char), execname_len, MAX_NAME_LENGTH) {
		if (*execname == '\0') break;
	}

	if (execname_len == MAX_NAME_LENGTH) {
		RETURN(EXEC_EXE_TOO_LONG);
	}

	char **argvec = *(char ***)(arg_addr + 1);
	SAFE_LOOP(argvec, sizeof(char *), argc, MAX_ARGUMENTS - 1) {
		if (*argvec == NULL) break;
		char *arg = *argvec;
		int arg_len;
		SAFE_LOOP(arg, sizeof(char), arg_len, MAX_ARG_LENGTH) {
			if (*arg == '\0') break;
		}

		if (arg_len == MAX_ARG_LENGTH) {
			RETURN(EXEC_ARG_TOO_LONG);
		}
	}

	/* execname is an argument too */
	argc++;

	if (argc == MAX_ARGUMENTS) {
		RETURN(EXEC_ARGS_TOO_LONG);
	}
}

