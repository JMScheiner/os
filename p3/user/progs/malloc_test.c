#include <malloc.h>
#include <stdio.h>
#include <syscall.h>
#include <rand.h>
#include <simics.h>

#define SIZE 100

int main(int argc, const char *argv[])
{
   int i, j;
   size_t random_sizes[SIZE];
   void* bases[SIZE];
   
   sgenrand(get_ticks());
   for (i = 0; i < SIZE; i++) random_sizes[i] = genrand() % 0x10000;

   for(i = 0; i < 20; i++)
   {
      for(j = 0; j < SIZE; j++)
      {
         lprintf("malloc'ing %d", random_sizes[j]);
         bases[j] = _malloc(random_sizes[j]);
      }
      
      for(j = 0; j < SIZE; j++)
      {
         lprintf("freeing %p", bases[j]);
         _free(bases[j]);
      }
   }

   new_pages((void*)0x2000000, 0x8000);
   remove_pages((void*)0x2000000);

   return 1234;
}



