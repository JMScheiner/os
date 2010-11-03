#include <simics.h>
#include <reg.h>
#include <asm_helper.h>
#include <exec2obj.h>
#include <vstring.h>
#include <string.h>

/**
 * @brief Halt the system
 *
 * Ceases execution of the operating system. The exact operation of this system 
 *    call depends on the kernel’s implementation and execution environment. 
 *    Kernels running under Simics should shut down the simulation via a call 
 *    to SIM halt(). However, implementations should be prepared to do 
 *    something reasonable if SIM halt() is a no-op, which will happen if the 
 *    kernel is run on real hardware.
 *
 * @param reg Ignored
 */
void halt_handler(volatile regstate_t reg)
{
   sim_halt();
   halt();
}

/** 
* @brief Fills in the user-specified buffer with the names of executable files 
*  stored in the system's RAM disk "file system" If there is enough room in 
*  the buffer for all of the (null-terminated) file names and an additional 
*  null byte after the last filename's terminating null, the system call will 
*  return the number of filenames successfully copied. Otherwise, an error 
*  code less than zero is returned and the contents of the buffer are 
*  undefined. For the curious among you, this system call is (very) loosely 
*  modeled on the System V getdents() call.
* 
* @param reg The register state on entry to the handler.
*/
void ls_handler(volatile regstate_t reg)
{
	char *arg_addr, *filename, *buf; 
   char zero = 0;
   int i;
   size_t total, copied, len;
	
   arg_addr = (char *)SYSCALL_ARG(reg);

	if(v_memcpy((char*)&len, arg_addr, sizeof(int)) < sizeof(int))
		RETURN(-1);
	
   if(v_memcpy((char*)&buf, arg_addr + sizeof(int), sizeof(char*)) < 
			sizeof(char*))
		RETURN(-1);
   
   total = 0;
   for(i = 0; i < exec2obj_userapp_count; i++)
   {
      filename = (char*)exec2obj_userapp_TOC[i].execname;
      lprintf(" Copying %s ", filename);
      
      copied = v_strcpy(buf, filename, len);
      
      /* If we couldn't copy the whole filename, return with failure. */
      if(copied < strlen(filename))
         RETURN(-1);
      
      len -= copied;
      buf += copied;
      total++;
   }
   
   
   v_memcpy(buf, &zero, 1);

   RETURN(total);
}


