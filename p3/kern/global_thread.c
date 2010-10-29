#include <kernel_types.h>
#include <global_thread.h>

static pcb_t _global_pcb;
static tcb_t _global_tcb;

pcb_t* global_pcb() { return &_global_pcb; } 
tcb_t* global_tcb() { return &_global_tcb; } 

