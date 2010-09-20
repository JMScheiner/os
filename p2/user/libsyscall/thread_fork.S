#include <syscall_int.h>

.globl thread_fork

/** @def thread_fork(void *(*func)(void *), void *arg, void *stack_base)
 *
 * @brief Fork a new thread running func(arg) on a stack beginning at
 *        stack_base
 *
 * @param func The function the new thread will start running
 *
 * @param arg The sole argument to func
 *
 * @param stack_base The base (highest address) of the stack for the child
 *        thread to use. This must be 32-bit aligned and ideally should
 *        be page aligned (last address of a page)
 *
 * @return The ID of the child thread on success, less than 0 on error
 */
thread_fork:
	int    $THREAD_FORK_INT  // Generate the trap
	testl  %eax, %eax        // Check the return value, we are the parent if non-zero
	jnz    PARENT            // Jump to the end if we are the parent thread
	movl   %esp, %ebp        // Store a reference to the stack pointer
	movl   16(%ebp), %esp    // Set the base of the child's stack frame
	pushl  12(%ebp)          // Push the argument for the child
	call   *8(%ebp)          // Execute the child's first function
	subl   -4, %esp          // Pop the argument
	pushl  %eax              // Save the return value
	call   thr_exit          // Exit with the return of the child's function
	// We should never reach here
PARENT:
	ret                      // %eax contains the return value

