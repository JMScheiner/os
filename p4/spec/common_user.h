/** 
* @file common_user.h
*
* @brief Definitions that use to be "kernel only" that are 
*  useful for handling software exceptions. 
*
* @author Justin Scheiner
* @author Tim Wilson
*/

#ifndef COMMON_USER_L4W9UHIX
#define COMMON_USER_L4W9UHIX

#define PAGE_MASK (PAGE_SIZE - 1)
#define PAGE_OF(addr) (((int)(addr)) & (~PAGE_MASK))

#define USER_STACK_BASE 0xc0000000
#define USER_STACK_START 0xb0000000

#define IDT_DE          0       /**< Devision Error (Fault) */
#define IDT_DB          1       /**< Debug Exception (Fault/Trap) */
#define IDT_NMI         2       /**< Non-Maskable Interrupt (Interrupt) */
#define IDT_BP          3       /**< Breakpoint (Trap) */
#define IDT_OF          4       /**< Overflow (Trap) */
#define IDT_BR          5       /**< BOUND Range exceeded (Fault) */
#define IDT_UD          6       /**< UnDefined Opcode (Fault) */
#define IDT_NM          7       /**< No Math coprocessor (Fault) 
                                     Device Not Available */
#define IDT_DF          8       /**< Double Fault (Abort) */
#define IDT_CSO         9       /**< Coprocessor Segment Overrun (Fault) */
#define IDT_TS          10      /**< Invalid Task Segment Selector (Fault) */
#define IDT_NP          11      /**< Segment Not Present (Fault) */
#define IDT_SS          12      /**< Stack Segment Fault (Fault) */
#define IDT_GP          13      /**< General Protection Fault (Fault) */
#define IDT_PF          14      /**< Page Fault (Fault) */
                                /* IDT entry 15 is reserved. */
#define IDT_MF          16      /**< X87 Math Fault (Fault) */
#define IDT_AC          17      /**< Alignment Check (Fault) */
#define IDT_MC          18      /**< Machine Check (Abort) */
#define IDT_XF          19      /**< SSE Floating Point Exception (Fault) */
                                /* IDT entries 16 through 31 are reserved. */
#define IDT_USER_START  32      /**< User define IDT entries start here */
#define IDT_ENTS        256     /* There are this many IDT entries */

#endif /* end of include guard: COMMON_USER_L4W9UHIX */

