
.globl atomic_xchg
.globl atomic_cmpxchg
.globl atomic_xadd
.globl atomic_add
.globl atomic_cmpset

atomic_xchg: 
	movl 	4(%esp), 	%eax		//Load &source into %eax
	movl  8(%esp), 	%edx 		//Load &destination into %edx
	movl	(%eax),		%ecx 		//Load source into %ecx
	
	xchg	%ecx, 		(%edx)	//Atomically exchange source into destination.
	movl	%ecx,			(%eax)	//Load swapped source back into source. 		
	ret

atomic_xadd: 
	movl 	4(%esp), 	%eax		//Load &source into %eax
	movl  8(%esp), 	%edx 		//Load &destination into %edx
	movl	(%eax),		%ecx 		//Load source into %ecx
	
	//Generates invalid opcode with lock#! Is this not valid!?
	lock xaddl	%ecx, (%edx)	//source = destination, destination = source + destination, 
	movl	%ecx,		(%eax)		//Load the result back into source.
	ret

atomic_add:
	movl  4(%esp), %edx        // Load dest into %edx
	movl  8(%esp), %eax        // Load src int %eax
	lock  xaddl  %eax, (%edx)  // eax = *dest, *dest += src;
	ret

atomic_cmpxchg: 
	movl	4(%esp), 	%ecx	//Load &source into %ecx
	movl 	8(%esp), 	%edx	//Load &destination into %edx.
	movl 	12(%esp), 	%eax	//Load comp into %eax.
	
	lock cmpxchg	%ecx, (%edx) //Exchange %ecx into (%edx), if (%edx) = %eax.
	
	//cmpxchg will set ZF on success.
	jnz atomic_cmpxchg_failed
	movl $0 , %eax				//Indicate success.
	ret

atomic_cmpxchg_failed:
	//%eax contains (%edx)
	movl 	%eax,	(%ecx) 	//Move the original value into &source.
	movl 	$-1,	%eax
	ret

atomic_cmpset:
	movl  4(%esp), %edx           // Load dest into %edx
	movl  8(%esp), %ecx           // Load src into %ecx
	movl  12(%esp), %eax          // Load comp into %eax
	lock  cmpxchg  %ecx, (%edx)   // if (*dest == comp) *dest = src
	jnz   failure
	movl  $0, %eax
	ret
failure:
	movl  $-1, %eax
	ret

