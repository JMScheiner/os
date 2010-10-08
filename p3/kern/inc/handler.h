#ifndef HANDLER_W0H6K1DA

#define HANDLER_W0H6K1DA

#include <stdint.h>
#include <seg.h>

/**** Trap Gate Struct : *******/

typedef uint8_t trap_gate_t[8];

#define IDT_TG_INIT(tg) 																		\
	{																									\
		tg[7] = 0x00;											/* offset[31:24]        */	\
		tg[6] = 0x00;											/* offset[23:16]        */	\
		tg[5] = 0x8e;								/* {P[1], DPL[2], 0, D[1], 110}	*/	\
		tg[4] = 0x00;											/* {0[3], reserved[5]}  */	\
		tg[3] = ((SEGSEL_KERNEL_CS & 0xFF00) >> 8);	/* segment[15:8]        */	\
		tg[2] = (SEGSEL_KERNEL_CS & 0x00FF);			/* segment[7:0]         */	\
		tg[1] = 0x00;											/* offset[15:8]         */	\
		tg[0] = 0x00;											/* offset[7:0]	         */	\
	}

#define IDT_TG_SET_OFFSET(tg, address)  												\
	{																			 					\
		tg[7] = (((uint32_t)(address) & 0xFF000000) >> 24); 						\
		tg[6] = (((uint32_t)(address) & 0x00FF0000) >> 16); 						\
		tg[1] = (((uint32_t)(address) & 0x0000FF00) >> 8); 						\
		tg[0] = ((uint32_t)(address) & 0x000000FF); 									\
	}

#define IDT_TG_SET_SS(tg, ss)				\
	{												\
		tg[3] = (((ss) & 0xFF00) >> 8); 	\
		tg[2] = ((ss) & 0x00FF); 			\
	}

#define INSTALL_HANDLER(tg, func, offset)             \
   IDT_TG_INIT(tg);                                   \
   IDT_TG_SET_OFFSET(tg, func);                       \
   IDT_TG_SET_SS(tg, SEGSEL_KERNEL_CS);               \
   memcpy(idt_base() + offset * sizeof(trap_gate_t),  \
      tg, sizeof(trap_gate_t));                        
   


int handler_install();

#endif /* end of include guard: HANDLER_W0H6K1DA */





