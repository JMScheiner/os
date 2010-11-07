#ifndef KVM_OMG6HOB6
#define KVM_OMG6HOB6

#include <kernel_types.h>

#define KVM_TOP      ((void*)(-3 * PAGE_SIZE))

/* Initialization */
void kvm_init();

/* Request */
int kvm_request_frames(int n_user, int n_kernel);

/* Allocation. */
int kvm_new_directory(pcb_t* pcb);
void* kvm_new_page(void);

/* Deallocation */
void kvm_free_page(void* page);

/* Utility */
void* kvm_vtop();
void* kvm_initial_table();

#endif /* end of include guard: KVM_OMG6HOB6 */

