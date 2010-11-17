#include <syscall_int.h>

/*
 * Exception handlers can be either assembly language or C.
 * This one is in assembly language so that it builds and runs
 * before you have written your swexn() stub routine, i.e.,
 * it is up to you to decide the best way to write this, and
 * the fact that this one is written in assembly language is not
 * a hint either way.
 */


/* This is NOT a real implementation, i.e., we just restart
 * the instruction that faulted, but this time without a
 * handler registered.  Note that some of this code is silly;
 * this is not an invitation for your submission to contain
 * silly code.
 */
.text
.globl _defswexn
_defswexn: /* called as _defswexn(void *arg, ureg_t *ureg) */
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	subl	$16,%esp		# space for syscall args
	movl	%esp,%esi		# arg pointer
	movl	$0,(%esi)		# esp3: 0
	movl	$0,4(%esi)		# eip: 0
	movl	$0,8(%esi)		# arg: 0
	movl	12(%ebp),%eax		# fetch: ureg (state when exception happened)
	movl	%eax,12(%esi)		# tell SWEXN: please (re)install that state
	int	$SWEXN_INT		# back we go to the faulting instruction
	movl	-4(%ebp),%esi
	leave
	ret				# more or less

/* Allocate some stack space for _defswexn() to run on */
.data
.space 5280
.globl _defswexn_stack
_defswexn_stack:
