#include <thread.h>
#include <syscall.h>
#include <simics.h>
#include <stdio.h>

#define STACK_SIZE 0x200
#define NTHREADS 5
void *child(void *param);

int main(int argc, const char *argv[])
{
   int i;
   thr_init(STACK_SIZE);
   
   for( i = 0; i < NTHREADS; i++)
      thr_create(child, NULL);
   
   return 0;
}

/** @brief Declare that we have run, then twiddle thumbs. */
void* child(void* param)
{
   int* mysanity = NULL;
   *mysanity = 0x0;
   return mysanity;
}



