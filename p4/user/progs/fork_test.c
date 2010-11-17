
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>
#include <syscall_int.h>


int main(int argc, const char *argv[])
{
   lprintf("My tid is 0x%x - I'm gonna fork!!!", gettid());
   
   int i;
   int pid = 0;
   for (i = 0; i < 50; i++)
   {
      pid = fork();
      if(pid == 0) break;
      else lprintf("Spawned child %x", pid);
   }

   lprintf("Child back - gettid returns 0x%x.", gettid());
   
   return 1;
}

