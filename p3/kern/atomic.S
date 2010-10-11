
.globl atomic_add

/** @def int atomic_add(int *dest, int src)
 *
 * @breif Perform *dest += src atomically
 *
 * @param dest The destination operand
 * @param src The source operand
 *
 * @return The original value of *dest
 */
atomic_add:
	movl        4(%esp), %edx     // Load dest into %edx
	movl        8(%esp), %eax     // Load src into %eax
	lock xaddl  %eax, (%edx)      // ret = *dest, *dest += src;
	ret                           // return ret

