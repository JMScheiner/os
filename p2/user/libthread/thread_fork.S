#include <syscall_int.h>

.globl thread_fork

/** @def thread_fork(void *(*func)(void *), void *arg, char *stack_base, 
 *                   tcb_t *tcb)
 *
 * @brief Fork a new thread running func(arg) on a stack beginning at
 *        stack_base
 *
 * @param func The function the new thread will start running
 *
 * @param arg The sole argument to func
 *
 * @param stack_base The base address of the stack the child will use
 *
 * @param tcb A partial thread control block the child will complete
 *
 * @return The ID of the child thread on success, less than 0 on error
 */
thread_fork:
	pushl  %ebp              // Save previous stack frame
	movl   %esp, %ebp        // Set up new stack frame
	int    $THREAD_FORK_INT  // Generate the trap
	testl  %eax, %eax        // Check the return value, we are the parent if non-zero
	jnz    PARENT            // Jump to the end if we are the parent thread
	movl   16(%ebp), %esp    // Move to the base of the child's stack frame
	pushl  %eax              // Save the return of THREAD_FORK_INT
	pushl  20(%ebp)          // Push the tcb of the child as an argument
	pushl  12(%ebp)          // Save arg on new stack.
	pushl  8(%ebp)          // Save func on new stack.
	call   thr_child_init    // Initialize the child and get a stack frame
	// We should never reach here
PARENT:
	pushl  %eax              // Save the return of THREAD_FORK_INT
	pushl  20(%ebp)          // Push the tcb of the child as an argument
	call   wait_for_child    // Wait for the child to initialize itself
	subl   $-4, %esp         // Pop the arguments
	popl   %eax              // Restore return of THREAD_FORK_INT 
	leave                    // Tear down stack frame
	ret

