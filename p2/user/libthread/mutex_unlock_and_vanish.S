.globl mutex_unlock_and_vanish

#include <syscall_int.h>

mutex_unlock_and_vanish:
	pushl	%ebp					//Push base pointer.
	movl	%esp, %ebp			//Move the base pointer to the old stack pointer.
	subl	$16, %esp			//Allocate local vars
	movl	8(%ebp), %eax		//Pull mutex_t* mp off the stack.
	movl	4(%eax), %eax		//Get mutex_node* next out of mp
	movl	%eax, -4(%ebp)		//Move next into a local variable.
	cmpl	$0, -4(%ebp) 		//Check if it is null.
	je	next_null				//If so jump to next_null

next_there:
	/****** Last stack access in this thread: ******/
	movl	-4(%ebp), %eax		//Grab next again.
	movl	$1, 12(%eax)		//Set next->cancel_deschedule true.
	movl  8(%eax), %esi		//Put next->tid in %esi - no need to save :P
	int   $MAKE_RUNNABLE_INT
	jmp	finish				//Jump to finish

next_null:
	/****** Last stack access in this thread: ******/
	movl	8(%ebp), %eax		//Get mp off the stack again.
	movl	$0, 12(%eax)		//Drop the "held" flag.

finish:
	int $VANISH_INT
