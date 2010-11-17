#ifndef _UREG_H_
#define _UREG_H_

typedef struct ureg_t {
	unsigned int cause; /* See <x86/idt.h> */
	unsigned int cr2;   /* Or else zero. */

	unsigned int ds;
	unsigned int es;
	unsigned int fs;
	unsigned int gs;

	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int zero;  /* Dummy %esp, set to zero */
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;

	unsigned int error_code;
	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
	unsigned int esp;
	unsigned int ss;
} ureg_t;

#endif /* _UREG_H_ */
