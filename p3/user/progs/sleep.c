
#include <syscall.h>
#include <simics.h>

int main(int argc, const char *argv[])
{
   if(fork()) while(1){}
   else{
      while(1)
      {
         sleep(100);
         lprintf("Back!");
      }
   }
   return 0;
}


