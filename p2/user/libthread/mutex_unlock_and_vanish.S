
.globl mutex_unlock_and_vanish

#include <syscall_int.h>

/* This is copied from mutex_unlock with an INT at the end */
mutex_unlock_and_vanish:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %ebx
	movl	$-1, %eax
	testl	%ebx, %ebx
	je	.L12
	movl	$-2, %eax
	cmpl	$0, 12(%ebx)
	je	.L12
	movl	$1, 8(%ebx)
	movl	$1, -8(%ebp)
	leal	20(%ebx), %eax
	movl	%eax, 4(%esp)
	leal	-8(%ebp), %eax
	movl	%eax, (%esp)
	call	atomic_xadd
	movl	16(%ebx), %eax
	cmpl	-8(%ebp), %eax
	je	.L14
.L18:
	movl	$-1, (%esp)
	call	yield
	movl	16(%ebx), %eax
	cmpl	-8(%ebp), %eax
	jne	.L18
.L14:
	movl	(%ebx), %eax
	testl	%eax, %eax
	je	.L16
	movl	(%eax), %eax
	movl	%eax, (%esp)
	call	make_runnable
.L16:
	addl	$1, 16(%ebx)
	movl	$0, %eax
.L12:
	addl	$12, %esp
	INT $VANISH_INT							//Trap to vanish on the int stack.

