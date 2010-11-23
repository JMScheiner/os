

#include <syscall.h>
#include <stdio.h>
#include <ureg.h>

#define STACKSIZE 0x200
#define COUNT 0x1000

char stack[STACKSIZE];
int eflags = 0;
int count = 0;

void handler(void* args, ureg_t* uregs)
{
   int ret;
   int saved_eflags;
   
   eflags++;
   saved_eflags = uregs->eflags;
   uregs->eflags = eflags;
   
   ret = swexn(stack, handler, 0, uregs);
   if( ret < 0 ) 
   {
      /* We have been shown to be evil! */
      uregs->eflags = saved_eflags;
      if(eflags % COUNT == 0) 
         printf("Still going... eflags = %x\n", eflags);
      swexn(stack + STACKSIZE, handler, 0, uregs);
   }
   else {
      printf(" Is %x really valid? ", eflags);
   }
}

int main(int argc, const char *argv[])
{
   unsigned char* magic, curious;
   swexn(stack + STACKSIZE, handler, 0, 0);
   
   magic = NULL;
   curious = *magic;
   return curious;
}

