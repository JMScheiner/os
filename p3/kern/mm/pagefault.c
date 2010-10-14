#include <reg.h>
#include <simics.h>
#include <process.h>

void page_fault_handler(regstate_t reg)
{
   // Determine the address that caused the fault. 
   // Search region list. 
   // Deploy the correct page fault handler for the region. 
}

void txt_fault(void* addr, int access_mode){}
void rodata_fault(void* addr, int access_mode){}
void dat_fault(void* addr, int access_mode){}
void bss_fault(void* addr, int access_mode){}
void stack_fault(void* addr, int access_mode){}

void txt_free(region_t* region){}
void rodata_free(region_t* region){}
void dat_free(region_t* region){}
void bss_free(region_t* region){}
void stack_free(region_t* region){}





