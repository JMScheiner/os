
#include <stdio.h>
#include <syscall.h>

static char string[] = "I'm in the .data section!";
void* function(void* v)
{
   printf("I'm a function!\n");
   return NULL;
}

int main(int argc, const char *argv[])
{
   char* addr = NULL;
   char test;
   int pid, status;

   if((pid = fork()) == 0)
   {
      addr[1] = 2;
   }
   else wait(&status);

   if((pid = fork()) == 0)
   {
      test = addr[2];
      printf("I shouldn't be able to print %c\n", test);
   }
   else wait(&status);
   
   if((pid = fork()) == 0)
   {
      test = addr[0x1000];
      printf("I also shouldn't be able to print %c\n", test);
   }
   else wait(&status);
      
   if((pid = fork()) == 0)
   {
      addr = (char*)function;
      addr[2] = '0';
   }
   else wait(&status);
   
   if((pid = fork()) == 0)
   {
      int zero = 0; 
      int one = 1;
      int undefined = one / zero;
      printf("NaN: %d", undefined);
   }
   else wait(&status);

   printf(string);

   return 0;
}



