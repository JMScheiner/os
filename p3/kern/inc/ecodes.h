#ifndef SYSCALL_CODES_FDKJ3489
#define SYSCALL_CODES_FDKJ3489

#define ESUCCESS  0     /* Generic success code. */
#define EFAIL     (-1)  /* Generic failure code. */
#define EARGS     (-2)  /* Invalid arguments. */
#define ELEN      (-3)  /* Invalid length */
#define EBUF      (-4)  /* Invalid user buffer. */
#define ENAME     (-5)  /* Invalid name (or tid / pid) */
#define EMULTHR   (-6)  /* Multiple threads when there shouldn't be. */
#define ECHILD    (-7)  /* No child status's to collect. */
#define ENOVM     (-8)  /* Out of virtual memory. */
#define ENOMEM    (-9)  /* Out of physical (direct mapped) memory */
#define ESTATE    (-10) /* System state is inconsistent with request. */

#endif

