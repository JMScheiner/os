#include <simics.h>
#include <reg.h>
#include <asm_helper.h>
#include <exec2obj.h>
#include <vstring.h>
#include <string.h>

/**
 * @brief Halt the system
 *
 * @param reg Ignored
 */
void halt_handler(volatile regstate_t reg)
{
   sim_halt();
   halt();
}

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


