#ifndef KVM_OMG6HOB6
#define KVM_OMG6HOB6

#include <kernel_types.h>

#define KVM_TOP      ((void*)(-3 * PAGE_SIZE))

void kvm_init();
int kvm_new_directory(pcb_t* pcb);
void* kvm_new_table(void* addr);
void* kvm_initial_table();

void* kvm_new_page(void);
void kvm_free_page(void* page);

/* This is the table that VIRTUAL_TABLE_PAGE and VIRTUAL_COPY_PAGE 
 * are mapped in.  V = P, and it is allocated in mm_init. */
void* kvm_vtop();

#endif /* end of include guard: KVM_OMG6HOB6 */

