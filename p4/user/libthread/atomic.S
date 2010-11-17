
.globl atomic_add

atomic_add:
   movl        4(%esp), %edx     // Load dest into %edx
   movl        8(%esp), %eax     // Load src into %eax
   lock xaddl  %eax, (%edx)      // ret = *dest, *dest += src;
   ret                           // return ret

