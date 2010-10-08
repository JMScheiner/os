
.globl invalidate_page

invalidate_page: 
   mov 4(%esp), %eax    // Grab the address from the stack. 
   invlpg (%eax)        // Invalidate the page at that address.
   ret


