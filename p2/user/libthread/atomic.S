
.globl atomic_xchg
.globl atomic_cmpxchg
.globl atomic_xadd

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
	
	lock# xadd	(%edx),	%ecx	//source = destination, destination = source + destination, 
	movl	%ecx,			(%eax)	//Load the result back into source.
	ret

atomic_cmpxchg: 
	//TODO Will write if we need it.


