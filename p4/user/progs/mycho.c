
#include <stdio.h>
#include <syscall.h>
#include <assert.h>
#include <simics.h>

static int N = 50;

static int nprogs = 4;
static char* progs[] = {"exec_basic", "fork_test1", "print_basic", "wild_test1"};

int main(int argc, const char *argv[])
{
   int i, j, pid;
   int status;
   for(i = 0; i < N; i++)
   {
      for(j = 0; j < nprogs; j++)
      {
         pid = fork();
         if(pid == 0)
         {
            lprintf("Just forked. ");
            char* args[2];
            args[0] = progs[j];
            args[1] = 0;
            exec(progs[j], args);
            assert(0);
         }
         else lprintf(" Back from fork. ");
      }
   }

   for(i = 0; i < N * nprogs; i++)
   {
      lprintf(" Calling wait. ");
      pid = wait(&status);
      if(pid < 0)
      {
         lprintf(" ********** FAILURE ********** ");
         MAGIC_BREAK;
      }
      lprintf("Collected status %d from process %d", status, pid);
   }


   return 0;
}


